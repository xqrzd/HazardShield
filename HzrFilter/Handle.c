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

#include "Handle.h"

FORCEINLINE BOOLEAN HndpFindAvailableHandle(
	_In_ PHANDLE_SYSTEM HandleSystem,
	_Out_ PULONG Handle)
{
	ULONG i;

	for (i = 0; i < HandleSystem->MaxHandles; i++)
	{
		if (HandleSystem->ObjectTable[i] == NULL)
		{
			*Handle = i;
			return TRUE;
		}
	}

	return FALSE;
}

FORCEINLINE NTSTATUS HndpGrowTable(
	_Inout_ PHANDLE_SYSTEM HandleSystem)
{
	NTSTATUS status;
	PVOID oldTable = HandleSystem->ObjectTable;
	PVOID newTable;
	SIZE_T oldTableSize;
	SIZE_T newTableSize;

	// Double table size.
	oldTableSize = HandleSystem->MaxHandles * sizeof(ULONG);
	newTableSize = oldTableSize * 2;

	newTable = ExAllocatePoolWithTag(PagedPool, newTableSize, 'HAND');

	if (newTable)
	{
		// Copy previous table entries.
		RtlCopyMemory(newTable, oldTable, oldTableSize);

		// Zero out new table entries.
		RtlZeroMemory(RtlOffsetToPointer(newTable, oldTableSize), oldTableSize);

		HandleSystem->ObjectTable = newTable;
		HandleSystem->MaxHandles *= 2;

		ExFreePoolWithTag(oldTable, 'HAND');

		status = STATUS_SUCCESS;
	}
	else
		status = STATUS_INSUFFICIENT_RESOURCES;

	return status;
}

NTSTATUS HndInitialize(
	_In_ PHANDLE_SYSTEM HandleSystem,
	_In_ ULONG InitialHandleCount)
{
	NTSTATUS status;
	SIZE_T tableSize = InitialHandleCount * sizeof(ULONG);

	HandleSystem->ObjectTable = ExAllocatePoolWithTag(PagedPool, tableSize, 'HAND');

	if (HandleSystem->ObjectTable)
	{
		RtlZeroMemory(HandleSystem->ObjectTable, tableSize);
		FltInitializePushLock(&HandleSystem->ObjectTableLock);
		HandleSystem->MaxHandles = InitialHandleCount;

		status = STATUS_SUCCESS;
	}
	else
		status = STATUS_INSUFFICIENT_RESOURCES;

	return status;
}

VOID HndFree(
	_In_ PHANDLE_SYSTEM HandleSystem)
{
	ExFreePoolWithTag(HandleSystem->ObjectTable, 'HAND');
	FltDeletePushLock(&HandleSystem->ObjectTableLock);
}

NTSTATUS HndCreateHandle(
	_In_ PHANDLE_SYSTEM HandleSystem,
	_In_ PVOID Object,
	_Out_ PULONG Handle)
{
	NTSTATUS status;

	if (!Object)
		return STATUS_INVALID_PARAMETER;

	FltAcquirePushLockExclusive(&HandleSystem->ObjectTableLock);

FindHandle:
	if (HndpFindAvailableHandle(HandleSystem, Handle))
	{
		HandleSystem->ObjectTable[*Handle] = Object;

		status = STATUS_SUCCESS;
	}
	else
	{
		DbgPrint("Need to grow handle table [Current size: %u]", HandleSystem->MaxHandles);

		status = HndpGrowTable(HandleSystem);

		if (NT_SUCCESS(status))
		{
			// Push locks cannot be acquired recursively, so use
			// goto instead of calling HndCreateHandle again.
			goto FindHandle;
		}
	}

	FltReleasePushLock(&HandleSystem->ObjectTableLock);

	return status;
}

NTSTATUS HndLookupObject(
	_In_ PHANDLE_SYSTEM HandleSystem,
	_In_ ULONG Handle,
	_Out_ PVOID* Object)
{
	NTSTATUS status;

	if (Handle < HandleSystem->MaxHandles)
	{
		PVOID object;

		FltAcquirePushLockShared(&HandleSystem->ObjectTableLock);

		object = HandleSystem->ObjectTable[Handle];

		if (object)
		{
			*Object = object;
			status = STATUS_SUCCESS;
		}
		else
			status = STATUS_NOT_FOUND;

		FltReleasePushLock(&HandleSystem->ObjectTableLock);
	}
	else
		status = STATUS_INVALID_PARAMETER;

	return status;
}

VOID HndReleaseHandle(
	_In_ PHANDLE_SYSTEM HandleSystem,
	_In_ ULONG Handle)
{
	if (Handle < HandleSystem->MaxHandles)
	{
		FltAcquirePushLockExclusive(&HandleSystem->ObjectTableLock);

		HandleSystem->ObjectTable[Handle] = NULL;

		FltReleasePushLock(&HandleSystem->ObjectTableLock);
	}
}