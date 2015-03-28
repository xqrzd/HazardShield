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

#pragma once

#include "Base.h"

typedef struct _HS_MEMORY_OBJECT {
	PVOID BaseAddress;
	SIZE_T Size;
	HANDLE ProcessId;

	DWORD Protection;
	DWORD Type;

	// Path
} HS_MEMORY_OBJECT, *PHS_MEMORY_OBJECT;

typedef struct _HS_MEMORY_PROVIDER *PHS_MEMORY_PROVIDER;

typedef VOID(*PHS_MEMORY_PROVIDER_CALLBACK)(
	_In_ PHS_MEMORY_PROVIDER Provider,
	_In_ PHS_MEMORY_OBJECT MemoryObject,
	_In_ PVOID Buffer,
	_In_ SIZE_T BufferSize
	);

typedef struct _HS_MEMORY_PROVIDER {
	PHS_MEMORY_PROVIDER_CALLBACK Callback;
	PVOID Context;

	HANDLE ProcessId;
} HS_MEMORY_PROVIDER, *PHS_MEMORY_PROVIDER;

__declspec(dllexport) BOOLEAN HzrVirtualQuery(
	_In_ PHS_MEMORY_PROVIDER Provider
	);