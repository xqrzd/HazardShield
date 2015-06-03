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

#include "ph.h"

#define KHS_PORT_NAME L"\\HzrFilterPort"

typedef struct _HS_FILE_INFO {
	PVOID Buffer;
	SIZE_T BufferSize;
} HS_FILE_INFO, *PHS_FILE_INFO;

typedef NTSTATUS(NTAPI *PHS_SCAN_FILE_ROUTINE)(
	_In_ PHS_FILE_INFO FileInfo,
	_Out_ PUCHAR ResponseFlags
	);

HRESULT KhsConnect(
	_In_ LPCWSTR PortName
	);

NTSTATUS KhsStartFiltering(
	_In_ DWORD NumberOfScanThreads,
	_In_ PHS_SCAN_FILE_ROUTINE FileScanRoutine
	);

VOID KhsDisconnect(
	);