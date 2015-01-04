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

	for (i = 0; i < MAX_HANDLE_COUNT; i++)
	{
		if (HandleSystem->Handles[i].Object == NULL)
		{
			*Handle = i;
			return TRUE;
		}
	}

	return FALSE;
}

VOID HndInitialize(
	_In_ PHANDLE_SYSTEM HandleSystem)
{
	RtlZeroMemory(HandleSystem->Handles, sizeof(HandleSystem->Handles));
	FltInitializePushLock(&HandleSystem->PushLock);
}

VOID HndFree(
	_In_ PHANDLE_SYSTEM HandleSystem)
{
	FltDeletePushLock(&HandleSystem->PushLock);
}

NTSTATUS HndCreateHandle(
	_In_ PHANDLE_SYSTEM HandleSystem,
	_In_ PVOID Object,
	_Out_ PULONG Handle)
{
	NTSTATUS status;

	FltAcquirePushLockExclusive(&HandleSystem->PushLock);

	if (HndpFindAvailableHandle(HandleSystem, Handle))
	{
		HandleSystem->Handles[*Handle].Object = Object;

		status = STATUS_SUCCESS;
	}
	else
		status = STATUS_INSUFFICIENT_RESOURCES;

	FltReleasePushLock(&HandleSystem->PushLock);

	return status;
}

NTSTATUS HndLookupObject(
	_In_ PHANDLE_SYSTEM HandleSystem,
	_In_ ULONG Handle,
	_Out_ PVOID* Object)
{
	NTSTATUS status;

	if (Handle < MAX_HANDLE_COUNT)
	{
		PVOID object;

		FltAcquirePushLockShared(&HandleSystem->PushLock);

		object = HandleSystem->Handles[Handle].Object;

		if (object)
		{
			*Object = object;
			status = STATUS_SUCCESS;
		}
		else
			status = STATUS_NOT_FOUND;

		FltReleasePushLock(&HandleSystem->PushLock);
	}
	else
		status = STATUS_INVALID_PARAMETER;

	return status;
}

VOID HndReleaseHandle(
	_In_ PHANDLE_SYSTEM HandleSystem,
	_In_ ULONG Handle)
{
	if (Handle < MAX_HANDLE_COUNT)
	{
		FltAcquirePushLockExclusive(&HandleSystem->PushLock);

		HandleSystem->Handles[Handle].Object = NULL;

		FltReleasePushLock(&HandleSystem->PushLock);
	}
}