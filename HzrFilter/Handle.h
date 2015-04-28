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
	for a system that rapidly uses and releases handles,
	so typically only a few handles will be used at once.

	Note that garbage collection is not supplied. Deleting
	a handle will not free the backing object, if there is one.
*/

#ifndef HZRFILTER_HANDLE_H
#define HZRFILTER_HANDLE_H

#include <fltKernel.h>

typedef struct _HS_HANDLE_SYSTEM {
	PVOID* ObjectTable;
	EX_PUSH_LOCK ObjectTableLock;
	ULONG MaxHandles;
} HS_HANDLE_SYSTEM, *PHS_HANDLE_SYSTEM;

/// <summary>
/// Initializes a new instance of HS_HANDLE_SYSTEM, with the given initial capacity.
/// </summary>
/// <param name="HandleSystem">Pointer to the HANDLE_SYSTEM to initialize.</param>
/// <param name="InitialHandleCount">The initial number of handles to allocate space for.</param>
NTSTATUS HsInitializeHandleSystem(
	_In_ PHS_HANDLE_SYSTEM HandleSystem,
	_In_ ULONG InitialHandleCount
	);

/// <summary>
/// Deletes the specified handle system.
/// </summary>
/// <param name="HandleSystem">Pointer to the HANDLE_SYSTEM to delete.</param>
VOID HsDeleteHandleSystem(
	_In_ PHS_HANDLE_SYSTEM HandleSystem
	);

/// <summary>
/// Creates a handle, which can be used to reference the given object.
/// </summary>
/// <param name="HandleSystem">Pointer to a HANDLE_SYSTEM.</param>
/// <param name="Object">The object this handle references.</param>
/// <param name="Handle">The handle which can be used to lookup the object.</param>
NTSTATUS HsCreateHandle(
	_In_ PHS_HANDLE_SYSTEM HandleSystem,
	_In_ PVOID Object,
	_Out_ PULONG Handle
	);

/// <summary>
/// Retrieves an object given a handle.
/// </summary>
/// <param name="HandleSystem">Pointer to a HANDLE_SYSTEM.</param>
/// <param name="Handle">A handle to an object to lookup.</param>
/// <param name="Object">The object represented by the handle.</param>
NTSTATUS HsLookupObjectByHandle(
	_In_ PHS_HANDLE_SYSTEM HandleSystem,
	_In_ ULONG Handle,
	_Out_ PVOID* Object
	);

/// <summary>
/// Removes a handle from the given HANDLE_SYSTEM.
/// </summary>
/// <param name="HandleSystem">Pointer to a HANDLE_SYSTEM.</param>
/// <param name="Handle">The handle to remove.</param>
VOID HsDeleteHandle(
	_In_ PHS_HANDLE_SYSTEM HandleSystem,
	_In_ ULONG Handle
	);

#endif