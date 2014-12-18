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
	UCHAR	BootCode[430];
	USHORT	EndMarker;		// 0xAA55
} NTFS_BOOT_SECTOR, *PNTFS_BOOT_SECTOR;
#pragma pack(pop)

typedef BOOLEAN NTFS_READ_SECTOR(
	_In_ struct _NTFS_VOLUME* NtfsVolume,
	_In_ ULONGLONG Sector,
	_In_ ULONG SectorCount,
	_Out_ PVOID Buffer
	);

typedef NTFS_READ_SECTOR *PNTFS_READ_SECTOR;

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