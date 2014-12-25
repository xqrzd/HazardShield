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
	NtfsInitializeListHead(&NtfsVolume->MftDataRuns);

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
		// TODO: walk through MFT data runs.
	}

	if (found)
	{
		// Read file record
		if (NtfsVolume->NtfsReadSector(NtfsVolume, startSector + relativeSector, sectorCount, FileRecord))
		{
			if (FileRecord->Magic == FILE_RECORD_MAGIC /*&& NtfsFileExists(FileRecord)*/)
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

	assert((attributeOffset + attribute->Size) < NtfsVolume->FileRecordSize);
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