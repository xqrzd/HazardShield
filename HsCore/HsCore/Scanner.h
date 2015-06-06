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
#include "clamav.h"

typedef struct _HS_SCANNER {
	struct cl_engine* ClamAvEngine;
	ULONG Signatures;
} HS_SCANNER, *PHS_SCANNER;

BOOLEAN HsInit(
	);

BOOLEAN HsCreateScanner(
	_Out_ PHS_SCANNER* Scanner
	);

VOID HsDeleteScanner(
	_In_ PHS_SCANNER Scanner
	);

cl_error_t HsLoadClamAvDatabase(
	_In_ PHS_SCANNER Scanner,
	_In_ PSTR Path,
	_In_ ULONG DatabaseOptions
	);

cl_error_t HsCompileClamAvDatabase(
	_In_ PHS_SCANNER Scanner
	);

cl_error_t HsScanBuffer(
	_In_ PHS_SCANNER Scanner,
	_In_ PVOID Buffer,
	_In_ SIZE_T Length,
	_In_ ULONG ScanOptions,
	_Out_ PCHAR* VirusName
	);