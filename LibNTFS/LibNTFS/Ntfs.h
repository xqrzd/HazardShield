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

#pragma once

#include <Windows.h>

// The first 17 file records
#define	MFT_RECORD_MFT			0
#define	MFT_RECORD_MFT_MIRROR	1
#define	MFT_RECORD_LOG_FILE		2
#define	MFT_RECORD_VOLUME		3
#define	MFT_RECORD_ATTR_DEF		4
#define	MFT_RECORD_ROOT			5
#define	MFT_RECORD_BITMAP		6
#define	MFT_RECORD_BOOT			7
#define	MFT_RECORD_BAD_CLUSTER	8
#define	MFT_RECORD_SECURE		9
#define	MFT_RECORD_UPCASE		10
#define	MFT_RECORD_EXTEND		11
#define	MFT_RECORD_RESERVED12	12
#define	MFT_RECORD_RESERVED13	13
#define	MFT_RECORD_RESERVED14	14
#define	MFT_RECORD_RESERVED15	15
#define	MFT_RECORD_USER			16

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
	union {
		ULONGLONG FileReference;
		struct
		{
			ULONGLONG RecordNumber : 48;
			ULONGLONG RecordSequence : 16;
		};
	};
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
	ULONG		FileSize;				// Real size of the file record
	ULONG		AllocSize;				// Allocated size of the file record
	NTFS_FILE_REFERENCE	BaseReference;	// File reference to the base file record
	USHORT		NextAttributeId;
	USHORT		Padding;				// Align to 4 byte boundary
	ULONG		RecordNumber;			// MFT record number
} NTFS_FILE_RECORD, *PNTFS_FILE_RECORD;

typedef BOOLEAN NTFS_READ_SECTOR(
	_In_ struct _NTFS_VOLUME* NtfsVolume,
	_In_ ULONGLONG Sector,
	_In_ ULONG SectorCount,
	_Out_ PVOID Buffer
	);

typedef NTFS_READ_SECTOR *PNTFS_READ_SECTOR;

#define NtfsOffsetToPointer(B,O)  ((PVOID)( ((PCHAR)(B)) + ((O))  ))

// Represents an NTFS Volume
typedef struct _NTFS_VOLUME {
	PNTFS_READ_SECTOR NtfsReadSector;
	USHORT	BytesPerSector;
	UCHAR	SectorsPerCluster;
	ULONG	FileRecordSize;	// File record size in bytes
	ULONG	IndexBlockSize;	// Index block size in bytes
	ULONGLONG MftStartSector; // Relative to the volume, not an absolute disk sector.

	PVOID Context;	// Can be used to store anything
} NTFS_VOLUME, *PNTFS_VOLUME;

BOOLEAN NtfsInitVolume(
	_In_ PNTFS_READ_SECTOR NtfsReadSector,
	_In_ USHORT BytesPerSector,
	_In_ PVOID Context,
	_Out_ PNTFS_VOLUME Volume
	);

BOOLEAN NtfsPatchUpdateSequence(
	_In_ PNTFS_VOLUME NtfsVolume,
	_Inout_ PUSHORT Sector,
	_In_ ULONG SectorCount,
	_In_ PUSHORT UsnAddress
	);

BOOLEAN NtfsReadFileRecord(
	_In_ PNTFS_VOLUME NtfsVolume,
	_In_ ULONG RecordNumber,
	_Out_ PNTFS_FILE_RECORD FileRecord
	);

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