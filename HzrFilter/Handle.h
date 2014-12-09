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

#ifndef HZRFILTER_HANDLE_H
#define HZRFILTER_HANDLE_H

#include <fltKernel.h>

#define MAX_HANDLE_COUNT 16

typedef struct _HANDLE_ENTRY {
	PVOID Object;
} HANDLE_ENTRY, *PHANDLE_ENTRY;

// TODO: Use a tree instead of a static array.
typedef struct _HANDLE_SYSTEM {
	EX_PUSH_LOCK PushLock;
	HANDLE_ENTRY Handles[MAX_HANDLE_COUNT];
} HANDLE_SYSTEM, *PHANDLE_SYSTEM;

VOID HndInitialize(
	_In_ PHANDLE_SYSTEM HandleSystem
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