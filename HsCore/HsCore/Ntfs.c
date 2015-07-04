/*
*  Copyright (C) 2015 Orbitech
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
	_In_opt_ PVOID Context,
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

				if (success)
					assert(FileRecord->BaseReference.HighRecordNumber == 0);
				else
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

	//printf("Attributes for file %u\n", FileRecord->RecordNumber);

	for (attributeOffset = FileRecord->AttributeOffset, attribute = NtfsOffsetToPointer(FileRecord, attributeOffset);
		((attributeOffset + attribute->Size) < NtfsVolume->FileRecordSize) && (attribute->Type != ATTR_TYPE_END);
		attributeOffset += attribute->Size, attribute = NtfsOffsetToPointer(attribute, attribute->Size))
	{
		// Always process attribute lists, since they may contain relevant attributes.
		if (attribute->Type == ATTR_TYPE_ATTRIBUTE_LIST)
		{
			//printf("Found attribute list in %u, %u\n", FileRecord->RecordNumber, attribute->NonResident);

			NtfsParseAttributeList(NtfsVolume, attribute, FileRecord, AttributeMask, ListHead);
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

		//printf("\tType: 0x%X, NonResident: %u, Flags: %u, Id: %u\n", attribute->Type, attribute->NonResident, attribute->Flags, attribute->Id);
	}

	//printf("\n");
}

BOOLEAN NtfspIsDuplicate(
	_In_ ULONG RecordNumber,
	_In_ PLIST_ENTRY ListHead)
{
	PLIST_ENTRY current;
	PLIST_ENTRY next;

	LIST_FOR_EACH_SAFE(current, next, ListHead)
	{
		if (CONTAINING_RECORD(current, NTFS_FILE_RECORD_ENTRY, ListEntry)->RecordNumber == RecordNumber)
			return TRUE;
	}

	return FALSE;
}

VOID NtfspAddDuplicate(
	_In_ ULONG RecordNumber,
	_In_ PLIST_ENTRY ListHead)
{
	PNTFS_FILE_RECORD_ENTRY entry = NtfsAllocate(sizeof(NTFS_FILE_RECORD_ENTRY));

	entry->RecordNumber = RecordNumber;

	NtfsInsertTailList(ListHead, &entry->ListEntry);
}

// TODO: Don't add duplicate entries
VOID NtfsParseAttributeList(
	_In_ PNTFS_VOLUME NtfsVolume,
	_In_ PNTFS_ATTRIBUTE AttributeList,
	_In_ PNTFS_FILE_RECORD FileRecord,
	_In_ ULONG AttributeMask,
	_Out_ PLIST_ENTRY ListHead)
{
	PNTFS_ATTRIBUTE_LIST originalAttributeList;
	ULONG bufferSize;

	if (NtfsReadAttributeData(NtfsVolume, AttributeList, &originalAttributeList, &bufferSize))
	{
		LIST_ENTRY duplicates;
		PNTFS_ATTRIBUTE_LIST attributeList;
		ULONG offset;

		NtfsInitializeListHead(&duplicates);

		for (attributeList = originalAttributeList, offset = 0;
			offset + attributeList->Size < AttributeList->Size;
			offset += attributeList->Size, attributeList = NtfsOffsetToPointer(attributeList, attributeList->Size))
		{
			assert(attributeList->BaseReference.HighRecordNumber == 0);

			// Skip contained attributes, and only read desired attributes.
			if (attributeList->BaseReference.RecordNumber != FileRecord->RecordNumber &&
				(ATTR_MASK(attributeList->Type) & AttributeMask))
			{
				// Hack-fix for skipping duplicate file records.
				if (!NtfspIsDuplicate(attributeList->BaseReference.RecordNumber, &duplicates))
				{
					PNTFS_FILE_RECORD fileRecord = NtfsAllocate(NtfsVolume->FileRecordSize);

					if (NtfsReadFileRecord(NtfsVolume, attributeList->BaseReference.RecordNumber, fileRecord))
						NtfsReadFileAttributes(NtfsVolume, fileRecord, AttributeMask, ListHead);

					NtfspAddDuplicate(attributeList->BaseReference.RecordNumber, &duplicates);

					NtfsFree(fileRecord);
				}
			}
		}

		NtfsFreeLinkedList(&duplicates, NTFS_FILE_RECORD_ENTRY, ListEntry);

		NtfsFree(originalAttributeList);
	}
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

		//printf("Data run [Sector: %llu] [Length: %llu, %llu MB]\n", dataRunEntry->SectorOffset, dataRunEntry->LengthInSectors, dataRunEntry->LengthInSectors / 2 / 1024);

		NtfsInsertTailList(ListHead, &dataRunEntry->ListEntry);

		lengthOffset = (PNTFS_LENGTH_OFFSET_BITFIELD)offset;
	}

	if (NonResidentAttribute->AllocSize != (totalLength * NtfsVolume->SectorsPerCluster * NtfsVolume->BytesPerSector))
		printf("NtfsGetDataRuns: Didn't read correct amount of data [Expected: %016llu] [Read: %016llu]\n", NonResidentAttribute->AllocSize, totalLength * NtfsVolume->SectorsPerCluster * NtfsVolume->BytesPerSector);

	//printf("\n");

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

BOOLEAN NtfsReadAttributeData(
	_In_ PNTFS_VOLUME NtfsVolume,
	_In_ PNTFS_ATTRIBUTE Attribute,
	_Out_ PVOID* Buffer,
	_Out_ PULONG BufferSize)
{
	BOOLEAN success = FALSE;

	if (Attribute->NonResident)
	{
		PNTFS_NONRESIDENT_ATTRIBUTE nonResidentAttribute = (PNTFS_NONRESIDENT_ATTRIBUTE)Attribute;
		PVOID nonResidentData = NtfsAllocate((SIZE_T)nonResidentAttribute->AllocSize);

		success = NtfsReadNonResidentAttributeData(NtfsVolume, nonResidentAttribute, nonResidentData);

		if (success)
		{
			*Buffer = nonResidentData;
			*BufferSize = (ULONG)nonResidentAttribute->RealSize;
		}
		else
			NtfsFree(nonResidentAttribute);
	}
	else
	{
		PNTFS_RESIDENT_ATTRIBUTE residentAttribute = (PNTFS_RESIDENT_ATTRIBUTE)Attribute;
		PVOID residentData = NtfsAllocate(residentAttribute->DataSize);

		RtlCopyMemory(residentData, NtfsReadResidentAttributeData(residentAttribute), residentAttribute->DataSize);

		*Buffer = residentData;
		*BufferSize = residentAttribute->DataSize;
		success = TRUE;
	}

	return success;
}

VOID NtfsWalkIndexEntries(
	_In_ PNTFS_VOLUME NtfsVolume,
	_In_ PNTFS_INDEX_ENTRY IndexEntry,
	_In_ ULONG TotalEntrySize,
	_In_ PVOID IndexAllocation,
	_In_ PNTFS_INDEX_ENTRY_CALLBACK IndexCallback,
	_In_ PVOID CallbackContext)
{
	PNTFS_INDEX_ENTRY indexEntry = IndexEntry;
	ULONG total = indexEntry->Size;

	while (total <= TotalEntrySize)
	{
		if (indexEntry->Flags & INDEX_ENTRY_FLAG_SUBNODE)
		{
			// Sub-node VCN is the last member in a variable size structure (why?)
			PULONGLONG subNodeVcn = NtfsOffsetToPointer(indexEntry, indexEntry->Size - sizeof(ULONGLONG));

			//printf("\nSub-node VCN: %llu\n", *subNodeVcn);
			NtfsGetIndexAllocationEntries(NtfsVolume, *subNodeVcn, IndexAllocation, IndexCallback, CallbackContext);
		}

		IndexCallback(NtfsVolume, indexEntry, CallbackContext);

		if (indexEntry->Flags & INDEX_ENTRY_FLAG_LAST)
			break;

		indexEntry = NtfsOffsetToPointer(indexEntry, indexEntry->Size);
		total += indexEntry->Size;
	}
}

BOOLEAN NtfsGetIndexAllocationEntries(
	_In_ PNTFS_VOLUME NtfsVolume,
	_In_ ULONGLONG VCN,
	_In_ PVOID IndexAllocation,
	_In_ PNTFS_INDEX_ENTRY_CALLBACK IndexCallback,
	_In_ PVOID CallbackContext)
{
	ULONG byteOffset = (ULONG)VCN * NtfsVolume->SectorsPerCluster * NtfsVolume->BytesPerSector;
	PNTFS_INDEX_BLOCK indexBlock = NtfsOffsetToPointer(IndexAllocation, byteOffset);
	PUSHORT usnAddress = NtfsOffsetToPointer(indexBlock, indexBlock->UpdateSequenceOffset);

	if (NtfsPatchUpdateSequence(NtfsVolume, (PUSHORT)indexBlock, NtfsVolume->IndexBlockSize / NtfsVolume->BytesPerSector, usnAddress))
	{
		PNTFS_INDEX_ENTRY indexEntry = NtfsOffsetToPointer(&indexBlock->IndexHeader, indexBlock->IndexHeader.EntryOffset);

		assert(indexBlock->Magic == INDEX_BLOCK_MAGIC);
		assert(indexBlock->VCN == VCN);

		NtfsWalkIndexEntries(NtfsVolume, indexEntry, indexBlock->IndexHeader.TotalEntrySize, IndexAllocation, IndexCallback, CallbackContext);

		return TRUE;
	}
	else
	{
		printf("NtfsGetIndexAllocationEntries: NtfsPatchUpdateSequence failed [VCN: %llu]\n", VCN);
		return FALSE;
	}
}

BOOLEAN NtfsGetIndexRootEntries(
	_In_ PNTFS_VOLUME NtfsVolume,
	_In_ PNTFS_INDEX_ROOT_ATTRIBUTE IndexRoot,
	_In_ PVOID IndexAllocation,
	_In_ PNTFS_INDEX_ENTRY_CALLBACK IndexCallback,
	_In_ PVOID CallbackContext)
{
	PNTFS_INDEX_ENTRY indexEntry;

	if (IndexRoot->Type != ATTR_TYPE_FILE_NAME)
	{
		printf("NtfsReadIndexRootEntries: Unsupported index root type %X\n", IndexRoot->Type);
		return FALSE;
	}

	indexEntry = NtfsOffsetToPointer(&IndexRoot->IndexHeader, IndexRoot->IndexHeader.EntryOffset);

	NtfsWalkIndexEntries(NtfsVolume, indexEntry, IndexRoot->IndexHeader.TotalEntrySize, IndexAllocation, IndexCallback, CallbackContext);

	return TRUE;
}

BOOLEAN NtfsEnumSubFiles(
	_In_ PNTFS_VOLUME NtfsVolume,
	_In_ ULONG RecordNumber,
	_In_ PNTFS_INDEX_ENTRY_CALLBACK IndexCallback,
	_In_ PVOID CallbackContext)
{
	BOOLEAN success = FALSE;
	PNTFS_FILE_RECORD fileRecord = NtfsAllocate(NtfsVolume->FileRecordSize);

	if (NtfsReadFileRecord(NtfsVolume, RecordNumber, fileRecord))
	{
		if (NtfsFileExists(fileRecord) && NtfsIsDirectory(fileRecord))
		{
			LIST_ENTRY attributes;
			PNTFS_ATTRIBUTE_ENTRY indexRootEntry;

			NtfsInitializeListHead(&attributes);

			NtfsReadFileAttributes(NtfsVolume, fileRecord, ATTR_MASK_INDEX_ROOT | ATTR_MASK_INDEX_ALLOCATION, &attributes);

			indexRootEntry = NtfsFindFirstAttribute(&attributes, ATTR_TYPE_INDEX_ROOT);

			if (indexRootEntry /*&& !indexRootEntry->Attribute->NonResident*/)
			{
				PNTFS_INDEX_ROOT_ATTRIBUTE indexRootAttribute = NtfsReadResidentAttributeData((PNTFS_RESIDENT_ATTRIBUTE)indexRootEntry->Attribute);

				PNTFS_ATTRIBUTE_ENTRY indexAllocationEntry = NtfsFindFirstAttribute(&attributes, ATTR_TYPE_INDEX_ALLOCATION);
				PVOID indexAllocationData = NULL;

				assert(!indexRootEntry->Attribute->NonResident);

				if (indexRootAttribute->IndexHeader.Flags & INDEX_HEADER_FLAGS_LARGE)
					assert(indexAllocationEntry);

				// Index allocation is optional
				if (indexAllocationEntry)
				{
					PNTFS_NONRESIDENT_ATTRIBUTE indexAllocation = (PNTFS_NONRESIDENT_ATTRIBUTE)indexAllocationEntry->Attribute;

					assert(indexAllocationEntry->Attribute->NonResident);
					assert(indexAllocation->AllocSize == indexAllocation->RealSize);

					indexAllocationData = NtfsAllocate((SIZE_T)indexAllocation->AllocSize);

					NtfsReadNonResidentAttributeData(NtfsVolume, indexAllocation, indexAllocationData);
				}

				success = NtfsGetIndexRootEntries(NtfsVolume, indexRootAttribute, indexAllocationData, IndexCallback, CallbackContext);

				if (indexAllocationData)
					NtfsFree(indexAllocationData);
			}
			else
				printf("NtfsEnumSubFiles: Unable to find valid index root\n");

			NtfsFreeLinkedList(&attributes, NTFS_ATTRIBUTE_ENTRY, ListEntry);
		}
		else
			printf("NtfsEnumSubFiles: Invalid file record [Record: %u] [Flags: %X]\n", RecordNumber, fileRecord->Flags);
	}
	else
		printf("NtfsEnumSubFiles: Unable to read file record %u\n", RecordNumber);

	NtfsFree(fileRecord);

	return success;
}

BOOLEAN NtfsReadFileDataStreams(
	_In_ PNTFS_VOLUME NtfsVolume,
	_In_ ULONG RecordNumber,
	_In_ PNTFS_STREAM_CALLBACK StreamCallback,
	_In_ PVOID CallbackContext)
{
	PNTFS_FILE_RECORD fileRecord = NtfsAllocate(NtfsVolume->FileRecordSize);

	if (NtfsReadFileRecord(NtfsVolume, RecordNumber, fileRecord))
	{
		LIST_ENTRY dataAttributes;
		PLIST_ENTRY current;
		PLIST_ENTRY next;

		NtfsInitializeListHead(&dataAttributes);

		NtfsReadFileAttributes(NtfsVolume, fileRecord, ATTR_MASK_DATA, &dataAttributes);

		LIST_FOR_EACH_SAFE(current, next, &dataAttributes)
		{
			PNTFS_ATTRIBUTE attribute = CONTAINING_RECORD(current, NTFS_ATTRIBUTE_ENTRY, ListEntry)->Attribute;
			PVOID buffer;
			ULONG bufferSize;

			if (NtfsReadAttributeData(NtfsVolume, attribute, &buffer, &bufferSize))
			{
				StreamCallback(NtfsVolume, attribute, buffer, bufferSize, CallbackContext);

				NtfsFree(buffer);
			}
		}

		NtfsFreeLinkedList(&dataAttributes, NTFS_ATTRIBUTE_ENTRY, ListEntry);
	}
	else
		printf("NtfsReadDataStreams: Unable to read file record %u\n", RecordNumber);

	NtfsFree(fileRecord);

	return FALSE;
}