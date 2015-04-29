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

#define HANDLE_TABLE_TAG 'tHzH'

/// <summary>
/// Finds an available handle.
/// </summary>
/// <param name="HandleSystem">Pointer to a HANDLE_SYSTEM.</param>
/// <param name="Handle">The next available handle.</param>
/// <returns>TRUE if an available handle is found, otherwise FALSE.</returns>
FORCEINLINE BOOLEAN HndpFindAvailableHandle(
	_In_ PHS_HANDLE_SYSTEM HandleSystem,
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

/// <summary>
/// Doubles the capacity of the given HANDLE_SYSTEM.
/// </summary>
/// <param name="HandleSystem">Pointer to a HANDLE_SYSTEM.</param>
/// <param name="NextFreeHandle">The next available handle.</param>
FORCEINLINE NTSTATUS HndpGrowTable(
	_Inout_ PHS_HANDLE_SYSTEM HandleSystem,
	_Out_ PULONG NextFreeHandle)
{
	NTSTATUS status;
	PVOID oldTable;
	PVOID newTable;
	SIZE_T oldTableSize;
	SIZE_T newTableSize;

	oldTable = HandleSystem->ObjectTable;
	oldTableSize = HandleSystem->MaxHandles * sizeof(PVOID);

	// Double table size.
	newTableSize = oldTableSize * 2;
	newTable = ExAllocatePoolWithTag(PagedPool, newTableSize, HANDLE_TABLE_TAG);

	if (newTable)
	{
		// Copy previous table entries.
		RtlCopyMemory(newTable, oldTable, oldTableSize);

		// Delete old table.
		ExFreePoolWithTag(oldTable, HANDLE_TABLE_TAG);

		// Zero out new table entries.
		RtlZeroMemory(RtlOffsetToPointer(newTable, oldTableSize), oldTableSize);

		// Next free handle is the old capacity.
		// Ex. If the old table had 3 handles, the
		// next free handle would be index[3].
		*NextFreeHandle = HandleSystem->MaxHandles;

		HandleSystem->ObjectTable = newTable;
		HandleSystem->MaxHandles *= 2;

		status = STATUS_SUCCESS;
	}
	else
		status = STATUS_INSUFFICIENT_RESOURCES;

	return status;
}

NTSTATUS HsInitializeHandleSystem(
	_In_ PHS_HANDLE_SYSTEM HandleSystem,
	_In_ ULONG InitialHandleCount)
{
	NTSTATUS status;
	SIZE_T tableSize = InitialHandleCount * sizeof(PVOID);

	HandleSystem->ObjectTable = ExAllocatePoolWithTag(PagedPool, tableSize, HANDLE_TABLE_TAG);

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

VOID HsDeleteHandleSystem(
	_In_ PHS_HANDLE_SYSTEM HandleSystem)
{
	ExFreePoolWithTag(HandleSystem->ObjectTable, HANDLE_TABLE_TAG);
	FltDeletePushLock(&HandleSystem->ObjectTableLock);
}

NTSTATUS HsCreateHandle(
	_In_ PHS_HANDLE_SYSTEM HandleSystem,
	_In_ PVOID Object,
	_Out_ PULONG Handle)
{
	NTSTATUS status;
	ULONG handle;

	if (!Object)
		return STATUS_INVALID_PARAMETER;

	FltAcquirePushLockExclusive(&HandleSystem->ObjectTableLock);

	if (HndpFindAvailableHandle(HandleSystem, &handle))
	{
		status = STATUS_SUCCESS;
	}
	else
	{
		DbgPrint("Need to grow handle table [Current size: %u]", HandleSystem->MaxHandles);

		status = HndpGrowTable(HandleSystem, &handle);
	}

	if (NT_SUCCESS(status))
	{
		HandleSystem->ObjectTable[handle] = Object;
		*Handle = handle;
	}

	FltReleasePushLock(&HandleSystem->ObjectTableLock);

	return status;
}

NTSTATUS HsLookupObjectByHandle(
	_In_ PHS_HANDLE_SYSTEM HandleSystem,
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

VOID HsDeleteHandle(
	_In_ PHS_HANDLE_SYSTEM HandleSystem,
	_In_ ULONG Handle)
{
	if (Handle < HandleSystem->MaxHandles)
	{
		FltAcquirePushLockExclusive(&HandleSystem->ObjectTableLock);

		HandleSystem->ObjectTable[Handle] = NULL;

		FltReleasePushLock(&HandleSystem->ObjectTableLock);
	}
}