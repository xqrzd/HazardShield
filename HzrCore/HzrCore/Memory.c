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

BOOLEAN HzrVirtualQuery(
	_In_ HANDLE ProcessId,
	_In_ PHZR_MEMORY_CALLBACK Callback)
{
	HANDLE processHandle;
	PUCHAR baseAddress;
	PVOID previousAllocationBase;
	MEMORY_BASIC_INFORMATION basicInfo;

	baseAddress = 0;
	previousAllocationBase = (PVOID)-1;

	processHandle = OpenProcess(
		PROCESS_QUERY_INFORMATION | PROCESS_VM_READ,
		FALSE,
		(DWORD)ProcessId);

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
		baseAddress += basicInfo.RegionSize;
	}

	CloseHandle(processHandle);

	return TRUE;
}