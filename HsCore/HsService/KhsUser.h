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
	PPH_STRING FileName;
} HS_FILE_INFO, *PHS_FILE_INFO;

// This response is the lack of any other flags.
// It cannot be combined with any other flags.
#define HS_RESPONSE_FLAG_CLEAN 0x0
#define HS_RESPONSE_FLAG_INFECTED 0x1
#define HS_RESPONSE_FLAG_DELETE 0x2

typedef UCHAR(NTAPI *PHS_SCAN_FILE_ROUTINE)(
	_In_ PHS_FILE_INFO FileInfo
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