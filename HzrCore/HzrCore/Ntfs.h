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

#pragma once

#include <Windows.h>

// The first 17 file records
#define MFT_RECORD_MFT			0
#define MFT_RECORD_MFT_MIRROR	1
#define MFT_RECORD_LOG_FILE		2
#define MFT_RECORD_VOLUME		3
#define MFT_RECORD_ATTR_DEF		4
#define MFT_RECORD_ROOT			5
#define MFT_RECORD_BITMAP		6
#define MFT_RECORD_BOOT			7
#define MFT_RECORD_BAD_CLUSTER	8
#define MFT_RECORD_SECURE		9
#define MFT_RECORD_UPCASE		10
#define MFT_RECORD_EXTEND		11
#define MFT_RECORD_RESERVED12	12
#define MFT_RECORD_RESERVED13	13
#define MFT_RECORD_RESERVED14	14
#define MFT_RECORD_RESERVED15	15
#define MFT_RECORD_USER			16

#pragma pack(push, 1)
typedef struct _NTFS_BOOT_SECTOR {
	UCHAR	JumpInstruction[3];
	UCHAR	OemId[8];		// NTFS
	USHORT	BytesPerSector;
	UCHAR	SectorsPerCluster;
	USHORT	ReservedSectors;
	UCHAR	Reserved[3];	// Always 0
	USHORT	NotUsed;
	UCHAR	MediaDescriptor;
	USHORT	Reserved2;		// Always 0
	USHORT	SectorsPerTrack;
	USHORT	NumberOfHeads;
	ULONG	HiddenSectors;
	ULONG	NotUsed2;
	ULONG	NotUsed3;
	ULONGLONG	TotalSectors;
	ULONGLONG	MFT_LCN;	// Master file table logical cluster number
	ULONGLONG	MFT_MIRROR_LCN;
	CHAR	ClustersPerFileRecord;
	UCHAR	NotUsed4[3];	// Always 0
	CHAR	ClustersPerIndexBlock;
	UCHAR	NotUsed5[3];	// Always 0
	UCHAR	VolumeSerialNumber[8];
	ULONG	Checksum;
	UCHAR	BootCode[426];
	USHORT	EndMarker;		// 0xAA55
} NTFS_BOOT_SECTOR, *PNTFS_BOOT_SECTOR;
#pragma pack(pop)

typedef struct _NTFS_FILE_REFERENCE {
	ULONG RecordNumber;
	USHORT HighRecordNumber;	// Not used?
	USHORT SequenceNumber;
} NTFS_FILE_REFERENCE, *PNTFS_FILE_REFERENCE;

#define	FILE_RECORD_MAGIC		'ELIF'
#define	FILE_RECORD_FLAG_INUSE	0x01	// File record is in use
#define	FILE_RECORD_FLAG_DIR	0x02	// File record is a directory

#define NtfsFileExists(FileRecord)	(FileRecord->Flags & FILE_RECORD_FLAG_INUSE)
#define NtfsIsDirectory(FileRecord)	(FileRecord->Flags & FILE_RECORD_FLAG_DIR)

typedef struct _NTFS_FILE_RECORD {
	ULONG		Magic;					// Always set to 'FILE'
	USHORT		UpdateSequenceOffset;
	USHORT		UpdateSequenceSize;		// Size in words of Update Sequence Number & Array
	ULONGLONG	LSN;					// $LogFile Sequence Number
	USHORT		SequenceNumber;
	USHORT		HardlinkCount;
	USHORT		AttributeOffset;		// Offset to the first attribute
	USHORT		Flags;
	ULONG		RealSize;				// Real size of the file record
	ULONG		AllocSize;				// Allocated size of the file record
	NTFS_FILE_REFERENCE	BaseReference;	// File reference to the base file record
	USHORT		NextAttributeId;
	USHORT		Padding;				// Align to 4 byte boundary
	ULONG		RecordNumber;			// MFT record number
} NTFS_FILE_RECORD, *PNTFS_FILE_RECORD;

#define ATTR_TYPE_STANDARD_INFORMATION	0x10
#define ATTR_TYPE_ATTRIBUTE_LIST		0x20
#define ATTR_TYPE_FILE_NAME				0x30
#define ATTR_TYPE_OBJECT_ID				0x40
#define ATTR_TYPE_SECURITY_DESCRIPTOR	0x50
#define ATTR_TYPE_VOLUME_NAME			0x60
#define ATTR_TYPE_VOLUME_INFORMATION	0x70
#define ATTR_TYPE_DATA					0x80
#define ATTR_TYPE_INDEX_ROOT			0x90
#define ATTR_TYPE_INDEX_ALLOCATION		0xA0
#define ATTR_TYPE_BITMAP				0xB0
#define ATTR_TYPE_REPARSE_POINT			0xC0
#define ATTR_TYPE_EA_INFORMATION		0xD0
#define ATTR_TYPE_EA					0xE0
#define ATTR_TYPE_LOGGED_UTILITY_STREAM	0x100
#define ATTR_TYPE_END					((ULONG)-1)

#define ATTR_FLAG_COMPRESSED	0x0001
#define ATTR_FLAG_ENCRYPTED		0x4000
#define ATTR_FLAG_SPARSE		0x8000

// Author: cyb70289
#define ATTR_INDEX(Attribute)	(((Attribute)>>4)-1)	// Attribute Type to index, eg. 0x10->0, 0x30->2
#define ATTR_MASK(Attribute)	(((ULONG)1)<<ATTR_INDEX(Attribute))	// Shift index to make bit flag

#define ATTR_MASK_STANDARD_INFORMATION	ATTR_MASK(ATTR_TYPE_STANDARD_INFORMATION)
#define ATTR_MASK_ATTRIBUTE_LIST		ATTR_MASK(ATTR_TYPE_ATTRIBUTE_LIST)
#define ATTR_MASK_FILE_NAME				ATTR_MASK(ATTR_TYPE_FILE_NAME)
#define ATTR_MASK_OBJECT_ID				ATTR_MASK(ATTR_TYPE_OBJECT_ID)
#define ATTR_MASK_SECURITY_DESCRIPTOR	ATTR_MASK(ATTR_TYPE_SECURITY_DESCRIPTOR)
#define ATTR_MASK_VOLUME_NAME			ATTR_MASK(ATTR_TYPE_VOLUME_NAME)
#define ATTR_MASK_VOLUME_INFORMATION	ATTR_MASK(ATTR_TYPE_VOLUME_INFORMATION)
#define ATTR_MASK_DATA					ATTR_MASK(ATTR_TYPE_DATA)
#define ATTR_MASK_INDEX_ROOT			ATTR_MASK(ATTR_TYPE_INDEX_ROOT)
#define ATTR_MASK_INDEX_ALLOCATION		ATTR_MASK(ATTR_TYPE_INDEX_ALLOCATION)
#define ATTR_MASK_BITMAP				ATTR_MASK(ATTR_TYPE_BITMAP)
#define ATTR_MASK_REPARSE_POINT			ATTR_MASK(ATTR_TYPE_REPARSE_POINT)
#define ATTR_MASK_EA_INFORMATION		ATTR_MASK(ATTR_TYPE_EA_INFORMATION)
#define ATTR_MASK_EA					ATTR_MASK(ATTR_TYPE_EA)
#define ATTR_MASK_LOGGED_UTILITY_STREAM	ATTR_MASK(ATTR_TYPE_LOGGED_UTILITY_STREAM)
#define ATTR_MASK_ALL					((ULONG)-1)

typedef	struct _NTFS_ATTRIBUTE {
	ULONG	Type;		// Attribute type
	ULONG	Size;		// Size in bytes (including this header)
	BOOLEAN	NonResident;
	UCHAR	NameLength;	// Name length in WCHARs
	USHORT	NameOffset;
	USHORT	Flags;
	USHORT	Id;			// Each attribute has a unique identifier (unique only to the file record)
} NTFS_ATTRIBUTE, *PNTFS_ATTRIBUTE;

typedef	struct _NTFS_RESIDENT_ATTRIBUTE {
	NTFS_ATTRIBUTE	Header;
	ULONG		DataSize;
	USHORT		DataOffset;
	UCHAR		IndexedFlag;
	UCHAR		Padding;
} NTFS_RESIDENT_ATTRIBUTE, *PNTFS_RESIDENT_ATTRIBUTE;

typedef struct _NTFS_NONRESIDENT_ATTRIBUTE {
	NTFS_ATTRIBUTE	Header;
	ULONGLONG	StartVCN;
	ULONGLONG	EndVCN;
	USHORT		DataRunOffset;
	USHORT		CompressionUnitSize;// Compression unit size = 2^x clusters. 0 implies uncompressed
	ULONG		Padding;
	ULONGLONG	AllocSize;			// This is the attribute size rounded up to the cluster size
	ULONGLONG	RealSize;
	ULONGLONG	InitializedSize;	// Initialized data size of the stream
} NTFS_NONRESIDENT_ATTRIBUTE, *PNTFS_NONRESIDENT_ATTRIBUTE;

typedef struct _NTFS_ATTRIBUTE_LIST {
	ULONG		Type;				// The attribute type
	USHORT		Size;
	UCHAR		NameLength;			// Name length in WCHARs
	UCHAR		NameOffset;
	ULONGLONG	StartVCN;			// Starting VCN, or zero if the attribute is resident
	NTFS_FILE_REFERENCE	BaseReference;	// Base File Reference of the attribute
	USHORT		Id;					// Attribute id
} NTFS_ATTRIBUTE_LIST, *PNTFS_ATTRIBUTE_LIST;

#define ATTR_FILENAME_FLAG_READONLY		0x00000001
#define ATTR_FILENAME_FLAG_HIDDEN		0x00000002
#define ATTR_FILENAME_FLAG_SYSTEM		0x00000004
#define ATTR_FILENAME_FLAG_ARCHIVE		0x00000020
#define ATTR_FILENAME_FLAG_DEVICE		0x00000040
#define ATTR_FILENAME_FLAG_NORMAL		0x00000080
#define ATTR_FILENAME_FLAG_TEMP			0x00000100
#define ATTR_FILENAME_FLAG_SPARSE		0x00000200
#define ATTR_FILENAME_FLAG_REPARSE		0x00000400
#define ATTR_FILENAME_FLAG_COMPRESSED	0x00000800
#define ATTR_FILENAME_FLAG_OFFLINE		0x00001000
#define ATTR_FILENAME_FLAG_NCI			0x00002000	// Not content indexed
#define ATTR_FILENAME_FLAG_ENCRYPTED	0x00004000
#define ATTR_FILENAME_FLAG_DIRECTORY	0x10000000
#define ATTR_FILENAME_FLAG_INDEXVIEW	0x20000000

#define ATTR_FILENAME_NAMESPACE_POSIX	0x00
#define ATTR_FILENAME_NAMESPACE_WIN32	0x01
#define ATTR_FILENAME_NAMESPACE_DOS		0x02

typedef struct _NTFS_FILENAME_ATTRIBUTE {
	NTFS_FILE_REFERENCE	ParentRecord;	// File reference to the parent directory
	ULONGLONG	CreationTime;
	ULONGLONG	AlterTime;
	ULONGLONG	MftChangeTime;
	ULONGLONG	ReadTime;
	ULONGLONG	AllocSize;		// Allocated size of the file
	ULONGLONG	RealSize;		// Real size of the file
	ULONG		Flags;
	ULONG		ER;				// Used by EAs and Reparse
	UCHAR		NameLength;		// Filename length WCHARs
	UCHAR		NameSpace;
	WCHAR		Name[1];		// Filename (not null terminated)
} NTFS_FILENAME_ATTRIBUTE, *PNTFS_FILENAME_ATTRIBUTE;

typedef struct _NTFS_ATTRIBUTE_ENTRY {
	PNTFS_ATTRIBUTE	Attribute;
	LIST_ENTRY ListEntry;
} NTFS_ATTRIBUTE_ENTRY, *PNTFS_ATTRIBUTE_ENTRY;

// From NTFS Forensics (by Jason Medeiros)
typedef struct _NTFS_LENGTH_OFFSET_BITFIELD {
	union {
		UCHAR Value;
		struct
		{
			UCHAR Length : 4;
			UCHAR Offset : 4;
		} Bitfield;
	};
} NTFS_LENGTH_OFFSET_BITFIELD, *PNTFS_LENGTH_OFFSET_BITFIELD;

typedef struct _NTFS_DATA_RUN_ENTRY {
	ULONGLONG SectorOffset;
	ULONGLONG LengthInSectors;
	LIST_ENTRY ListEntry;
} NTFS_DATA_RUN_ENTRY, *PNTFS_DATA_RUN_ENTRY;

typedef struct _NTFS_FILE_RECORD_ENTRY {
	ULONG RecordNumber;
	LIST_ENTRY ListEntry;
} NTFS_FILE_RECORD_ENTRY, *PNTFS_FILE_RECORD_ENTRY;

#define INDEX_HEADER_FLAGS_SMALL 0x00	// Fits in Index Root
#define INDEX_HEADER_FLAGS_LARGE 0x01	// Index Allocation needed

typedef struct _NTFS_INDEX_HEADER {
	ULONG	EntryOffset;	// Offset to first Index Entry, relative to this address.
	ULONG	TotalEntrySize;	// Total size of the index entries
	ULONG	AllocEntrySize;	// Allocated size of index entries
	UCHAR	Flags;
	UCHAR	Padding[3];
} NTFS_INDEX_HEADER, *PNTFS_INDEX_HEADER;

typedef struct _NTFS_INDEX_ROOT_ATTRIBUTE {
	ULONG		Type;				// Attribute type (ATTR_TYPE_FILE_NAME: Directory, 0: Index View)
	ULONG		CollationRule;
	ULONG		Size;				// Size of Index Allocation Entry in bytes
	UCHAR		IndexRecordSize;	// In clusters
	UCHAR		Padding[3];
	NTFS_INDEX_HEADER IndexHeader;
} NTFS_INDEX_ROOT_ATTRIBUTE, *PNTFS_INDEX_ROOT_ATTRIBUTE;

#define INDEX_ENTRY_FLAG_SUBNODE	0x01	// This entry contains a sub-node
#define INDEX_ENTRY_FLAG_LAST		0x02	// The index entry does not represent a file but it can point to a sub - node

typedef struct _NTFS_INDEX_ENTRY {
	NTFS_FILE_REFERENCE	FileReference;	// MFT Reference of the file
	USHORT	Size;						// Size of index entry
	USHORT	FileNameOffset;
	UCHAR	Flags;
	UCHAR	Padding[3];
	NTFS_FILENAME_ATTRIBUTE FileName;
	// VCN of index allocation with sub-nodes [Offset = Base + Size - 8]
	// VCN only exists if Flags & INDEX_ENTRY_FLAG_SUBNODE
} NTFS_INDEX_ENTRY, *PNTFS_INDEX_ENTRY;

#define INDEX_BLOCK_MAGIC	'XDNI'

typedef struct _NTFS_INDEX_BLOCK {
	ULONG		Magic;				// "INDX"
	USHORT		UpdateSequenceOffset;
	USHORT		UpdateSequenceSize;	// Size in words of the Update Sequence Number & Array
	ULONGLONG	LSN;				// $LogFile sequence number
	ULONGLONG	VCN;				// VCN of this INDX buffer in the Index Allocation
	NTFS_INDEX_HEADER IndexHeader;
} NTFS_INDEX_BLOCK, *PNTFS_INDEX_BLOCK;

typedef VOID(*PNTFS_INDEX_ENTRY_CALLBACK)(
	_In_ struct _NTFS_VOLUME* NtfsVolume,
	_In_ PNTFS_INDEX_ENTRY IndexEntry,
	_In_ PVOID Context
	);

typedef VOID(*PNTFS_STREAM_CALLBACK)(
	_In_ struct _NTFS_VOLUME* NtfsVolume,
	_In_ PNTFS_ATTRIBUTE Attribute,
	_In_ PVOID Buffer,
	_In_ ULONG BufferSize,
	_In_ PVOID Context
	);

// Notes: If Sector is 0, only BytesPerSector and Context will be valid in NtfsVolume
typedef BOOLEAN(*PNTFS_READ_SECTOR)(
	_In_ struct _NTFS_VOLUME* NtfsVolume,
	_In_ ULONGLONG Sector,
	_In_ ULONG SectorCount,
	_Out_ PVOID Buffer
	);

// Represents an NTFS Volume
typedef struct _NTFS_VOLUME {
	PNTFS_READ_SECTOR NtfsReadSector;
	USHORT	BytesPerSector;
	UCHAR	SectorsPerCluster;
	ULONG	FileRecordSize;	// File record size in bytes
	ULONG	IndexBlockSize;	// Index block size in bytes
	ULONGLONG MftStartSector; // Relative to the volume, not an absolute disk sector.
	LIST_ENTRY MftDataRuns;

	PVOID Context;	// Can be used to store anything
} NTFS_VOLUME, *PNTFS_VOLUME;

__declspec(dllexport) BOOLEAN NtfsInitVolume(
	_In_ PNTFS_READ_SECTOR NtfsReadSector,
	_In_ USHORT BytesPerSector,
	_In_ PVOID Context,
	_Out_ PNTFS_VOLUME Volume
	);

__declspec(dllexport) VOID NtfsFreeVolume(
	_In_ PNTFS_VOLUME NtfsVolume
	);

__declspec(dllexport) BOOLEAN NtfsReadFileRecord(
	_In_ PNTFS_VOLUME NtfsVolume,
	_In_ ULONG RecordNumber,
	_Out_ PNTFS_FILE_RECORD FileRecord
	);

__declspec(dllexport) BOOLEAN NtfsPatchUpdateSequence(
	_In_ PNTFS_VOLUME NtfsVolume,
	_Inout_ PUSHORT Sector,
	_In_ ULONG SectorCount,
	_In_ PUSHORT UsnAddress
	);

__declspec(dllexport) VOID NtfsReadFileAttributes(
	_In_ PNTFS_VOLUME NtfsVolume,
	_In_ PNTFS_FILE_RECORD FileRecord,
	_In_ ULONG AttributeMask,
	_Out_ PLIST_ENTRY ListHead
	);

__declspec(dllexport) VOID NtfsParseAttributeList(
	_In_ PNTFS_VOLUME NtfsVolume,
	_In_ PNTFS_ATTRIBUTE AttributeList,
	_In_ PNTFS_FILE_RECORD FileRecord,
	_In_ ULONG AttributeMask,
	_Out_ PLIST_ENTRY ListHead
	);

__declspec(dllexport) PNTFS_ATTRIBUTE_ENTRY NtfsFindFirstAttribute(
	_In_ PLIST_ENTRY ListHead,
	_In_ ULONG AttributeType
	);

__declspec(dllexport) PNTFS_ATTRIBUTE_ENTRY NtfsFindNextAttribute(
	_In_ PLIST_ENTRY ListHead,
	_In_ PLIST_ENTRY StartEntry,
	_In_ ULONG AttributeType
	);

__declspec(dllexport) VOID NtfsGetDataRuns(
	_In_ PNTFS_VOLUME NtfsVolume,
	_In_ PNTFS_NONRESIDENT_ATTRIBUTE NonResidentAttribute,
	_Out_ PLIST_ENTRY ListHead
	);

__declspec(dllexport) BOOLEAN NtfsReadDataRuns(
	_In_ PNTFS_VOLUME NtfsVolume,
	_In_ PLIST_ENTRY DataRunsHead,
	_Out_ PVOID Buffer
	);

__declspec(dllexport) PVOID NtfsReadResidentAttributeData(
	_In_ PNTFS_RESIDENT_ATTRIBUTE Attribute
	);

__declspec(dllexport) BOOLEAN NtfsReadNonResidentAttributeData(
	_In_ PNTFS_VOLUME NtfsVolume,
	_In_ PNTFS_NONRESIDENT_ATTRIBUTE Attribute,
	_Out_ PVOID Buffer
	);

__declspec(dllexport) BOOLEAN NtfsReadAttributeData(
	_In_ PNTFS_VOLUME NtfsVolume,
	_In_ PNTFS_ATTRIBUTE Attribute,
	_Out_ PVOID* Buffer,
	_Out_ PULONG BufferSize
	);

__declspec(dllexport) VOID NtfsWalkIndexEntries(
	_In_ PNTFS_VOLUME NtfsVolume,
	_In_ PNTFS_INDEX_ENTRY IndexEntry,
	_In_ ULONG TotalEntrySize,
	_In_ PVOID IndexAllocation,
	_In_ PNTFS_INDEX_ENTRY_CALLBACK IndexCallback,
	_In_ PVOID CallbackContext
	);

__declspec(dllexport) BOOLEAN NtfsGetIndexAllocationEntries(
	_In_ PNTFS_VOLUME NtfsVolume,
	_In_ ULONGLONG VCN,
	_In_ PVOID IndexAllocation,
	_In_ PNTFS_INDEX_ENTRY_CALLBACK IndexCallback,
	_In_ PVOID CallbackContext
	);

__declspec(dllexport) BOOLEAN NtfsGetIndexRootEntries(
	_In_ PNTFS_VOLUME NtfsVolume,
	_In_ PNTFS_INDEX_ROOT_ATTRIBUTE IndexRoot,
	_In_ PVOID IndexAllocation,
	_In_ PNTFS_INDEX_ENTRY_CALLBACK IndexCallback,
	_In_ PVOID CallbackContext
	);

__declspec(dllexport) BOOLEAN NtfsEnumSubFiles(
	_In_ PNTFS_VOLUME NtfsVolume,
	_In_ ULONG RecordNumber,
	_In_ PNTFS_INDEX_ENTRY_CALLBACK IndexCallback,
	_In_ PVOID CallbackContext
	);

__declspec(dllexport) BOOLEAN NtfsReadFileDataStreams(
	_In_ PNTFS_VOLUME NtfsVolume,
	_In_ ULONG RecordNumber,
	_In_ PNTFS_STREAM_CALLBACK StreamCallback,
	_In_ PVOID CallbackContext
	);

#define NtfsOffsetToPointer(Base, Offset)	((PVOID)(((PCHAR)(Base)) + ((Offset))))

PVOID FORCEINLINE NtfsAllocate(
	_In_ SIZE_T Size)
{
#ifdef _DEBUG
	return malloc(Size);
#else
	return HeapAlloc(GetProcessHeap(), HEAP_GENERATE_EXCEPTIONS, Size);
#endif
}

VOID FORCEINLINE NtfsFree(
	_In_ PVOID Buffer)
{
#ifdef _DEBUG
	free(Buffer);
#else
	HeapFree(GetProcessHeap(), 0, Buffer);
#endif
}

FORCEINLINE ULONG NtfsClustersToBytes(
	_In_ CHAR NumberOfClusters,
	_In_ USHORT BytesPerSector,
	_In_ UCHAR SectorsPerCluster)
{
	// If the number is positive(up to 0x7F), then it represents clusters per MFT record.
	// If the number is negative(0x80 to 0xFF), then the size of the file record is 2 raised to the absolute value of this number.

	if (NumberOfClusters > 0)
		return BytesPerSector * SectorsPerCluster * NumberOfClusters;
	else
		return 1 << (-NumberOfClusters);
}

FORCEINLINE BOOLEAN NtfsIsVolumeValid(
	_In_ PNTFS_BOOT_SECTOR BootSector)
{
	return BootSector->OemId[0] == 'N'
		&& BootSector->OemId[1] == 'T'
		&& BootSector->OemId[2] == 'F'
		&& BootSector->OemId[3] == 'S'
		&& BootSector->EndMarker == 0xAA55;
}

#define LIST_FOR_EACH_SAFE(Current, Next, Head) for (Current = (Head)->Flink, Next = Current->Flink; Current != (Head); Current = Next, Next = Current->Flink)

FORCEINLINE VOID NtfsInitializeListHead(
	_Out_ PLIST_ENTRY ListHead)
{
	ListHead->Flink = ListHead->Blink = ListHead;
}

FORCEINLINE VOID NtfsInsertTailList(
	_Inout_ PLIST_ENTRY ListHead,
	_Inout_ PLIST_ENTRY Entry)
{
	PLIST_ENTRY OldBlink;
	OldBlink = ListHead->Blink;
	Entry->Flink = ListHead;
	Entry->Blink = OldBlink;
	OldBlink->Flink = Entry;
	ListHead->Blink = Entry;
}

#define NtfsFreeLinkedList(Head, Type, Field) { \
	PLIST_ENTRY Current, Next; \
	LIST_FOR_EACH_SAFE(Current, Next, Head) { \
		Type* structure = CONTAINING_RECORD(Current, Type, Field); \
		NtfsFree(structure); \
	} \
}