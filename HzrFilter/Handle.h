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

/*
	A simple handle system that maps a handle to a pointer.
	In this case an array is used, as this module was designed
	to support a system that rapidly uses and releases handles,
	so typically only a few handles will be used at once.
*/

#ifndef HZRFILTER_HANDLE_H
#define HZRFILTER_HANDLE_H

#include <fltKernel.h>

typedef struct _HANDLE_SYSTEM {
	PVOID* ObjectTable;
	EX_PUSH_LOCK ObjectTableLock;
	ULONG MaxHandles;
} HANDLE_SYSTEM, *PHANDLE_SYSTEM;

NTSTATUS HndInitialize(
	_In_ PHANDLE_SYSTEM HandleSystem,
	_In_ ULONG InitialHandleCount
	);

VOID HndFree(
	_In_ PHANDLE_SYSTEM HandleSystem
	);

NTSTATUS HndCreateHandle(
	_In_ PHANDLE_SYSTEM HandleSystem,
	_In_ PVOID Object,
	_Out_ PULONG Handle
	);

NTSTATUS HndLookupObject(
	_In_ PHANDLE_SYSTEM HandleSystem,
	_In_ ULONG Handle,
	_Out_ PVOID* Object
	);

VOID HndReleaseHandle(
	_In_ PHANDLE_SYSTEM HandleSystem,
	_In_ ULONG Handle
	);

#endif