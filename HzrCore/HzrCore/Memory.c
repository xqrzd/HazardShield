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

#include "Memory.h"

#define HS_MAX_MEMORY_SCAN_SIZE 52428800

BOOLEAN HzrVirtualQuery(
	_In_ PHS_MEMORY_PROVIDER Provider)
{
	HANDLE processHandle;
	PUCHAR baseAddress;
	MEMORY_BASIC_INFORMATION basicInfo;

	baseAddress = 0;

	processHandle = OpenProcess(
		PROCESS_QUERY_INFORMATION | PROCESS_VM_READ,
		FALSE,
		(DWORD)Provider->ProcessId);

	if (!processHandle)
	{
		return FALSE;
	}

	while (VirtualQueryEx(
		processHandle,
		baseAddress,
		&basicInfo,
		sizeof(basicInfo)))
	{
		if (basicInfo.State == MEM_COMMIT &&
			FlagOff(basicInfo.Protect, PAGE_GUARD | PAGE_NOACCESS) &&
			basicInfo.RegionSize <= HS_MAX_MEMORY_SCAN_SIZE)
		{
			BOOL success;
			PVOID buffer;
			SIZE_T bytesRead;

			buffer = HsAllocate(basicInfo.RegionSize);

			success = ReadProcessMemory(
				processHandle,
				baseAddress,
				buffer,
				basicInfo.RegionSize,
				&bytesRead);

			if (success)
			{
				HS_MEMORY_OBJECT memoryObject;

				memoryObject.BaseAddress = basicInfo.BaseAddress;
				memoryObject.Size = basicInfo.RegionSize;
				memoryObject.ProcessId = Provider->ProcessId;
				memoryObject.Protection = basicInfo.Protect;
				memoryObject.Type = basicInfo.Type;

				Provider->Callback(Provider, &memoryObject, buffer, basicInfo.RegionSize);
			}

			HsFree(buffer);
		}

		baseAddress += basicInfo.RegionSize;
	}

	CloseHandle(processHandle);

	return TRUE;
}