/*
* Copyright (C) 2015 Orbitech
*
* Authors: xqrzd
*
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License version 2 as
* published by the Free Software Foundation.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program; if not, write to the Free Software
* Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
* MA 02110-1301, USA.
*/

#include "Ntfs.h"
#include <stdio.h>
#ifdef _DEBUG
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>
#endif

VOID StreamRead(
	_In_ struct _NTFS_VOLUME* NtfsVolume,
	_In_ PNTFS_ATTRIBUTE Attribute,
	_In_ PVOID Buffer,
	_In_ ULONG BufferSize)
{
	/*printf("Data: %s\n", Buffer);

	if (Attribute->NameOffset && Attribute->NameLength)
	{
		WCHAR streamName[MAX_PATH];

		RtlCopyMemory(streamName, NtfsOffsetToPointer(Attribute, Attribute->NameOffset), Attribute->NameLength * sizeof(WCHAR));
		streamName[Attribute->NameLength] = L'\0';

		wprintf(L"Name: %s\n", streamName);
	}*/
}

BOOLEAN ReadSector(
	_In_ struct _NTFS_VOLUME* NtfsVolume,
	_In_ ULONGLONG Sector,
	_In_ ULONG SectorCount,
	_Out_ PVOID Buffer)
{
	BOOL result;
	HANDLE volumeHandle = NtfsVolume->Context;
	LARGE_INTEGER byteOffset;
	DWORD bytesToRead;
	DWORD bytesRead;
	bytesToRead = SectorCount * NtfsVolume->BytesPerSector;
	byteOffset.QuadPart = Sector * NtfsVolume->BytesPerSector;
	SetFilePointerEx(volumeHandle, byteOffset, NULL, FILE_BEGIN);
	result = ReadFile(volumeHandle, Buffer, bytesToRead, &bytesRead, NULL);
	return result;
}

VOID IndexCallback(
	_In_ struct _NTFS_VOLUME* NtfsVolume,
	_In_ PNTFS_INDEX_ENTRY IndexEntry)
{
	if (IndexEntry->FileNameOffset &&
		IndexEntry->FileName.NameLength &&
		IndexEntry->FileName.NameSpace != ATTR_FILENAME_NAMESPACE_DOS)
	{
		WCHAR filePath[MAX_PATH];
		RtlCopyMemory(filePath, IndexEntry->FileName.Name, IndexEntry->FileName.NameLength * sizeof(WCHAR));
		filePath[IndexEntry->FileName.NameLength] = L'\0';
		
		if (IndexEntry->FileReference.RecordNumber == 2022)
			NtfsEnumSubFiles(NtfsVolume, IndexEntry->FileReference.RecordNumber);
		if (IndexEntry->FileReference.RecordNumber == 3838)
			NtfsEnumSubFiles(NtfsVolume, IndexEntry->FileReference.RecordNumber);

		if (IndexEntry->FileName.RealSize <= 8388608 && IndexEntry->FileReference.RecordNumber > 17)
		{
			wprintf(L"%u\t %s\n", IndexEntry->FileReference.RecordNumber, filePath);
			NtfsReadFileDataStreams(NtfsVolume, IndexEntry->FileReference.RecordNumber);
		}
	}
}

void main()
{
	HANDLE volumeHandle = CreateFileW(L"\\\\.\\C:", GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (volumeHandle != INVALID_HANDLE_VALUE)
	{
		NTFS_VOLUME ntfsVolume;
		NtfsInitVolume(ReadSector, IndexCallback, StreamRead, 512, volumeHandle, &ntfsVolume);
		NtfsEnumSubFiles(&ntfsVolume, MFT_RECORD_ROOT);
		CloseHandle(volumeHandle);
		NtfsFreeVolume(&ntfsVolume);
	}
	else
		printf("CreateFileW failed %u\n", GetLastError());
#ifdef _DEBUG
	_CrtDumpMemoryLeaks();
#endif
	getchar();
}