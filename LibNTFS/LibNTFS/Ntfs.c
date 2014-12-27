/*
*  Copyright (C) 2014 Orbitech
*
*  Authors: xqrzd
*
*  This program is free software; you can redistribute it and/or modify
*  it under the terms of the GNU General Public License version 2 as
*  published by the Free Software Foundation.
*
*  This program is distributed in the hope that it will be useful,
*  but WITHOUT ANY WARRANTY; without even the implied warranty of
*  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*  GNU General Public License for more details.
*
*  You should have received a copy of the GNU General Public License
*  along with this program; if not, write to the Free Software
*  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
*  MA 02110-1301, USA.
*/

#include "Ntfs.h"
#include <assert.h>
#include <stdio.h>

BOOLEAN NtfsInitVolume(
	_In_ PNTFS_READ_SECTOR NtfsReadSector,
	_In_ USHORT BytesPerSector,
	_In_ PVOID Context,
	_Out_ PNTFS_VOLUME NtfsVolume)
{
	BOOLEAN success = FALSE;

	PNTFS_BOOT_SECTOR bootSector = NtfsAllocate(BytesPerSector);

	NtfsVolume->NtfsReadSector = NtfsReadSector;
	NtfsVolume->BytesPerSector = BytesPerSector;
	NtfsVolume->Context = Context;

	if (NtfsReadSector(NtfsVolume, 0, 1, bootSector))
	{
		if (NtfsIsVolumeValid(bootSector))
		{
			PNTFS_FILE_RECORD mftFileRecord;

			NtfsVolume->BytesPerSector = bootSector->BytesPerSector;
			NtfsVolume->SectorsPerCluster = bootSector->SectorsPerCluster;
			NtfsVolume->FileRecordSize = NtfsClustersToBytes(bootSector->ClustersPerFileRecord, bootSector->BytesPerSector, bootSector->SectorsPerCluster);
			NtfsVolume->IndexBlockSize = NtfsClustersToBytes(bootSector->ClustersPerIndexBlock, bootSector->BytesPerSector, bootSector->SectorsPerCluster);
			NtfsVolume->MftStartSector = bootSector->MFT_LCN * bootSector->SectorsPerCluster;

			printf("Init NTFS Volume\n");
			printf("\tBytesPerSector: %u\n", NtfsVolume->BytesPerSector);
			printf("\tSectorsPerCluster: %u\n", NtfsVolume->SectorsPerCluster);
			printf("\tFile record size: %u\n", NtfsVolume->FileRecordSize);
			printf("\tIndex block size: %u\n", NtfsVolume->IndexBlockSize);
			printf("\tMFT start sector: %llu\n\n", NtfsVolume->MftStartSector);

			mftFileRecord = NtfsAllocate(NtfsVolume->FileRecordSize);

			if (NtfsReadFileRecord(NtfsVolume, MFT_RECORD_MFT, mftFileRecord))
			{
				LIST_ENTRY attributes;
				PNTFS_ATTRIBUTE_ENTRY dataEntry;

				NtfsInitializeListHead(&attributes);

				NtfsReadFileAttributes(NtfsVolume, mftFileRecord, ATTR_MASK_DATA, &attributes);

				dataEntry = NtfsFindFirstAttribute(&attributes, ATTR_TYPE_DATA);

				if (dataEntry)
				{
					if (dataEntry->Attribute->NonResident)
					{
						NtfsInitializeListHead(&NtfsVolume->MftDataRuns);

						NtfsGetDataRuns(NtfsVolume, (PNTFS_NONRESIDENT_ATTRIBUTE)dataEntry->Attribute, &NtfsVolume->MftDataRuns);

						success = TRUE;
					}
					else
						printf("NtfsInitVolume: MFT data attribute is resident\n");
				}
				else
					printf("NtfsInitVolume: Unable to find MFT data attribute\n");

				NtfsFreeLinkedList(&attributes, NTFS_ATTRIBUTE_ENTRY, ListEntry);
			}
			else
				printf("NtfsInitVolume: Unable to read MFT file record\n");

			NtfsFree(mftFileRecord);
		}
		else
			printf("NtfsInitVolume: Volume isn't NTFS [OemId: %s] [EndMarker: %X]\n", bootSector->OemId, bootSector->EndMarker);
	}
	else
		printf("NtfsInitVolume: NtfsReadSector failed to read boot sector\n");

	NtfsFree(bootSector);

	return success;
}

VOID NtfsFreeVolume(
	_In_ PNTFS_VOLUME NtfsVolume)
{
	NtfsFreeLinkedList(&NtfsVolume->MftDataRuns, NTFS_DATA_RUN_ENTRY, ListEntry);
}

// Notes: FileRecord must be at least the size of NTFS_VOLUME.FileRecordSize
BOOLEAN NtfsReadFileRecord(
	_In_ PNTFS_VOLUME NtfsVolume,
	_In_ ULONG RecordNumber,
	_Out_ PNTFS_FILE_RECORD FileRecord)
{
	BOOLEAN success = FALSE;
	BOOLEAN found = FALSE;

	ULONG sectorCount = NtfsVolume->FileRecordSize / NtfsVolume->BytesPerSector;
	ULONGLONG relativeSector = RecordNumber * sectorCount; // Relative to the data run it belongs to
	ULONGLONG startSector = 0;

	if (RecordNumber <= MFT_RECORD_USER)
	{
		startSector = NtfsVolume->MftStartSector;
		found = TRUE;
	}
	else
	{
		PLIST_ENTRY current;
		PLIST_ENTRY next;

		LIST_FOR_EACH_SAFE(current, next, &NtfsVolume->MftDataRuns)
		{
			PNTFS_DATA_RUN_ENTRY dataRunEntry = CONTAINING_RECORD(current, NTFS_DATA_RUN_ENTRY, ListEntry);

			// Does this run contain the desired record?
			if (relativeSector <= dataRunEntry->LengthInSectors)
			{
				startSector = dataRunEntry->SectorOffset;
				found = TRUE;
				break;
			}
			else
				relativeSector -= dataRunEntry->LengthInSectors;
		}
	}

	if (found)
	{
		// Read file record
		if (NtfsVolume->NtfsReadSector(NtfsVolume, startSector + relativeSector, sectorCount, FileRecord))
		{
			if (FileRecord->Magic == FILE_RECORD_MAGIC)
			{
				PUSHORT usnAddress = NtfsOffsetToPointer(FileRecord, FileRecord->UpdateSequenceOffset);

				success = NtfsPatchUpdateSequence(NtfsVolume, (PUSHORT)FileRecord, sectorCount, usnAddress);

				if (!success)
					printf("NtfsReadFileRecord: NtfsPatchUpdateSequence failed for record %u\n", RecordNumber);
			}
			else
				printf("NtfsReadFileRecord: File record %u is invalid\n", RecordNumber);
		}
		else
			printf("NtfsReadFileRecord: Unable to find record %u\n", RecordNumber);
	}
	else
		printf("NtfsReadFileRecord: Unable to find record %u\n", RecordNumber);

	return success;
}

BOOLEAN NtfsPatchUpdateSequence(
	_In_ PNTFS_VOLUME NtfsVolume,
	_Inout_ PUSHORT Sector,
	_In_ ULONG SectorCount,
	_In_ PUSHORT UsnAddress)
{
	USHORT usn = *UsnAddress;
	PUSHORT usArray = UsnAddress + 1;
	ULONG i;

	for (i = 0; i < SectorCount; i++)
	{
		Sector += ((NtfsVolume->BytesPerSector >> 1) - 1);

		if (*Sector != usn)
			return FALSE;

		*Sector = usArray[i]; // Write back correct data.
		Sector++;
	}

	return TRUE;
}

VOID NtfsReadFileAttributes(
	_In_ PNTFS_VOLUME NtfsVolume,
	_In_ PNTFS_FILE_RECORD FileRecord,
	_In_ ULONG AttributeMask,
	_Out_ PLIST_ENTRY ListHead)
{
	PNTFS_ATTRIBUTE attribute;
	ULONG attributeOffset;

	attributeOffset = FileRecord->AttributeOffset;
	attribute = NtfsOffsetToPointer(FileRecord, FileRecord->AttributeOffset);

	printf("Attributes for file %u\n", FileRecord->RecordNumber);

	while (attribute->Type != ATTR_TYPE_END && ((attributeOffset + attribute->Size) < NtfsVolume->FileRecordSize))
	{
		// Always process attribute lists, since they may contain relevant attributes.
		if (attribute->Type == ATTR_TYPE_ATTRIBUTE_LIST)
		{
			// TODO: parse attribute list.
		}
		// Only read desired attributes.
		else if (ATTR_MASK(attribute->Type) & AttributeMask)
		{
			// Allocate memory for the entry and the attribute data itself.
			PNTFS_ATTRIBUTE_ENTRY attributeEntry = NtfsAllocate(sizeof(NTFS_ATTRIBUTE_ENTRY) + attribute->Size);

			// Set the attribute to point towards the extra allocated space (after the structure)
			attributeEntry->Attribute = NtfsOffsetToPointer(attributeEntry, sizeof(NTFS_ATTRIBUTE_ENTRY));
			RtlCopyMemory(attributeEntry->Attribute, attribute, attribute->Size);

			// Add attribute to linked list
			NtfsInsertTailList(ListHead, &attributeEntry->ListEntry);
		}

		printf("\tType: 0x%X, NonResident: %u, Flags: %u\n", attribute->Type, attribute->NonResident, attribute->Flags);

		attributeOffset += attribute->Size;
		attribute = NtfsOffsetToPointer(attribute, attribute->Size);
	}

	printf("\n");

	//assert((attributeOffset + attribute->Size) < NtfsVolume->FileRecordSize);
}

PNTFS_ATTRIBUTE_ENTRY NtfsFindFirstAttribute(
	_In_ PLIST_ENTRY ListHead,
	_In_ ULONG AttributeType)
{
	PLIST_ENTRY current;
	PLIST_ENTRY next;

	LIST_FOR_EACH_SAFE(current, next, ListHead)
	{
		PNTFS_ATTRIBUTE_ENTRY attributeEntry = CONTAINING_RECORD(current, NTFS_ATTRIBUTE_ENTRY, ListEntry);

		if (attributeEntry->Attribute->Type == AttributeType)
			return attributeEntry;
	}

	return NULL;
}

PNTFS_ATTRIBUTE_ENTRY NtfsFindNextAttribute(
	_In_ PLIST_ENTRY ListHead,
	_In_ PLIST_ENTRY StartEntry,
	_In_ ULONG AttributeType)
{
	return NULL;
}

// Notes: The caller must eventually call NtfsFreeLinkedList on ListHead
VOID NtfsGetDataRuns(
	_In_ PNTFS_VOLUME NtfsVolume,
	_In_ PNTFS_NONRESIDENT_ATTRIBUTE NonResidentAttribute,
	_Out_ PLIST_ENTRY ListHead)
{
	PUCHAR offset = NtfsOffsetToPointer(NonResidentAttribute, NonResidentAttribute->DataRunOffset);
	PNTFS_LENGTH_OFFSET_BITFIELD lengthOffset = (PNTFS_LENGTH_OFFSET_BITFIELD)offset;

	LONGLONG absoluteDataRunOffset = 0;	// Used to keep track of the absolute data run offset, since they are relative to the previous one
	ULONGLONG totalLength = 0;

	while (lengthOffset->Value)
	{
		PNTFS_DATA_RUN_ENTRY dataRunEntry;
		ULONGLONG dataRunLength = 0; // Length of the current data run in clusters.
		LONGLONG dataRunOffset = 0; // Offset of the current data run in clusters (this value is relative to the previous data run).

		offset += sizeof(NTFS_LENGTH_OFFSET_BITFIELD);

		if (lengthOffset->Bitfield.Length == 0 || lengthOffset->Bitfield.Length > 8 ||
			lengthOffset->Bitfield.Offset > 8)
		{
			printf("NtfsGetDataRuns: length_offset bitfield is corrupted [Length: %u] [Offset: %u]\n", lengthOffset->Bitfield.Length, lengthOffset->Bitfield.Offset);
			return;
		}

		// Read length of data run
		RtlCopyMemory(&dataRunLength, offset, lengthOffset->Bitfield.Length);

		offset += lengthOffset->Bitfield.Length;

		if (lengthOffset->Bitfield.Offset) // Not sparse file
		{
			// Check sign bit
			if (offset[lengthOffset->Bitfield.Offset - 1] & 0x80)
				dataRunOffset = -1;

			// Read location of data run (relative to the previous data run)
			RtlCopyMemory(&dataRunOffset, offset, lengthOffset->Bitfield.Offset);

			offset += lengthOffset->Bitfield.Offset;
		}
		else
			printf("NtfsGetDataRuns: Found sparse file\n");

		totalLength += dataRunLength;
		absoluteDataRunOffset += dataRunOffset;

		if (absoluteDataRunOffset < 0)
		{
			printf("NtfsGetDataRuns: totalDataOffset is corrupted: %lld\n", absoluteDataRunOffset);
			return;
		}

		dataRunEntry = NtfsAllocate(sizeof(NTFS_DATA_RUN_ENTRY));

		// Data runs are in clusters, convert them to sectors
		dataRunEntry->SectorOffset = absoluteDataRunOffset * NtfsVolume->SectorsPerCluster;
		dataRunEntry->LengthInSectors = dataRunLength * NtfsVolume->SectorsPerCluster;

		printf("Data run [Sector: %llu] [Length: %llu, %llu MB]\n", dataRunEntry->SectorOffset, dataRunEntry->LengthInSectors, dataRunEntry->LengthInSectors / 2 / 1024);

		NtfsInsertTailList(ListHead, &dataRunEntry->ListEntry);

		lengthOffset = (PNTFS_LENGTH_OFFSET_BITFIELD)offset;
	}

	if (NonResidentAttribute->AllocSize != (totalLength * NtfsVolume->SectorsPerCluster * NtfsVolume->BytesPerSector))
		printf("NtfsGetDataRuns: Didn't read correct amount of data [Expected: %016llu] [Read: %016llu]\n", NonResidentAttribute->AllocSize, totalLength * NtfsVolume->SectorsPerCluster * NtfsVolume->BytesPerSector);

	printf("\n");

	assert(NonResidentAttribute->AllocSize == (totalLength * NtfsVolume->SectorsPerCluster * NtfsVolume->BytesPerSector));
}

// Notes: Buffer must be at least the size of the data runs
BOOLEAN NtfsReadDataRuns(
	_In_ PNTFS_VOLUME NtfsVolume,
	_In_ PLIST_ENTRY DataRunsHead,
	_Out_ PVOID Buffer)
{
	ULONGLONG position = 0;

	PLIST_ENTRY current;
	PLIST_ENTRY next;

	LIST_FOR_EACH_SAFE(current, next, DataRunsHead)
	{
		PNTFS_DATA_RUN_ENTRY dataRunEntry = CONTAINING_RECORD(current, NTFS_DATA_RUN_ENTRY, ListEntry);

		if (!NtfsVolume->NtfsReadSector(
			NtfsVolume,
			dataRunEntry->SectorOffset,
			(ULONG)dataRunEntry->LengthInSectors,
			(PUCHAR)Buffer + position))
		{
			return FALSE;
		}

		position += dataRunEntry->LengthInSectors * NtfsVolume->BytesPerSector;
	}

	return TRUE;
}

PVOID NtfsReadResidentAttributeData(
	_In_ PNTFS_RESIDENT_ATTRIBUTE Attribute)
{
	return NtfsOffsetToPointer(Attribute, Attribute->DataOffset);
}

BOOLEAN NtfsReadNonResidentAttributeData(
	_In_ PNTFS_VOLUME NtfsVolume,
	_In_ PNTFS_NONRESIDENT_ATTRIBUTE Attribute,
	_Out_ PVOID Buffer)
{
	BOOLEAN success = FALSE;
	LIST_ENTRY dataRuns;

	NtfsInitializeListHead(&dataRuns);

	// Attribute isn't stored within the file record.
	NtfsGetDataRuns(NtfsVolume, Attribute, &dataRuns);

	success = NtfsReadDataRuns(NtfsVolume, &dataRuns, Buffer);

	NtfsFreeLinkedList(&dataRuns, NTFS_DATA_RUN_ENTRY, ListEntry);

	return success;
}