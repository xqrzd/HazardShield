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

#include "clamav.h"
#include "Memory.h"

typedef VOID(*PHS_MEMORY_OBJECT_CALLBACK)(
	_In_ PHS_MEMORY_OBJECT MemoryObject,
	_In_ PCHAR VirusName
	);

typedef struct _HZR_SCANNER {
	struct cl_engine* Engine;
	ULONG Signatures;

	// Scanner callbacks.
	PHS_MEMORY_OBJECT_CALLBACK MemoryObjectCallback;
} HZR_SCANNER, *PHZR_SCANNER;

__declspec(dllexport) cl_error_t HzrInitClamAv(
	);

__declspec(dllexport) BOOLEAN HzrCreateScanner(
	_Out_ PHZR_SCANNER* Scanner,
	_In_ PHS_MEMORY_OBJECT_CALLBACK MemoryObjectCallback
	);

__declspec(dllexport) VOID HzrDeleteScanner(
	_In_ PHZR_SCANNER Scanner
	);

__declspec(dllexport) cl_error_t HzrLoadClamAvDatabase(
	_In_ PHZR_SCANNER Scanner,
	_In_ CONST PCHAR Path,
	_In_ ULONG DatabaseOptions
	);

__declspec(dllexport) cl_error_t HzrCompileClamAvDatabase(
	_In_ PHZR_SCANNER Scanner
	);

__declspec(dllexport) cl_error_t HzrScanBuffer(
	_In_ PHZR_SCANNER Scanner,
	_In_ CONST PVOID Buffer,
	_In_ SIZE_T Length,
	_In_ ULONG ScanOptions,
	_Out_ PCHAR* VirusName
	);

__declspec(dllexport) BOOLEAN HsScanProcessMemoryBasic(
	_In_ PHZR_SCANNER Scanner,
	_In_ HANDLE ProcessId
	);