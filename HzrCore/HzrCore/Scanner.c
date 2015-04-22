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

#include "Memory.h"
#include "Scanner.h"

cl_error_t HzrInitClamAv()
{
	return cl_init(CL_INIT_DEFAULT);
}

BOOLEAN HzrCreateScanner(
	_Out_ PHZR_SCANNER* Scanner,
	_In_ PHS_MEMORY_OBJECT_CALLBACK MemoryObjectCallback)
{
	PHZR_SCANNER scanner = HsAllocate(sizeof(HZR_SCANNER));

	scanner->Engine = cl_engine_new();

	if (scanner->Engine)
	{
		scanner->Signatures = 0;

		scanner->MemoryObjectCallback = MemoryObjectCallback;

		//HzrLoadClamAvDatabase(scanner, "C:\\ProgramData\\Hazard Shield", CL_DB_BYTECODE);

		//HzrCompileClamAvDatabase(scanner);

		*Scanner = scanner;

		return TRUE;
	}

	HsFree(scanner);

	return FALSE;
}

VOID HzrDeleteScanner(
	_In_ PHZR_SCANNER Scanner)
{
	cl_engine_free(Scanner->Engine);
	HsFree(Scanner);
}

cl_error_t HzrLoadClamAvDatabase(
	_In_ PHZR_SCANNER Scanner,
	_In_ CONST PCHAR Path,
	_In_ ULONG DatabaseOptions)
{
	return cl_load(
		Path,
		Scanner->Engine,
		&Scanner->Signatures,
		DatabaseOptions);
}

cl_error_t HzrCompileClamAvDatabase(
	_In_ PHZR_SCANNER Scanner)
{
	return cl_engine_compile(Scanner->Engine);
}

cl_error_t HzrScanBuffer(
	_In_ PHZR_SCANNER Scanner,
	_In_ CONST PVOID Buffer,
	_In_ SIZE_T Length,
	_In_ ULONG ScanOptions,
	_Out_ PCHAR* VirusName)
{
	cl_error_t result;
	struct cl_fmap* map;

	map = cl_fmap_open_memory(Buffer, Length);

	if (map)
	{
		result = cl_scanmap_callback(
			map,
			VirusName,
			NULL,
			Scanner->Engine,
			ScanOptions,
			NULL);

		cl_fmap_close(map);
	}
	else
		result = CL_EMAP;

	return result;
}

VOID HsMemoryCallback(
	_In_ PHS_MEMORY_PROVIDER Provider,
	_In_ PHS_MEMORY_OBJECT MemoryObject,
	_In_ PVOID Buffer,
	_In_ SIZE_T BufferSize)
{
	PHZR_SCANNER scanner;
	cl_error_t result;
	PCHAR virusName;

	scanner = Provider->Context;
	virusName = NULL;

	result = HzrScanBuffer(
		scanner,
		Buffer,
		MemoryObject->Size,
		CL_SCAN_STDOPT,
		&virusName);

	scanner->MemoryObjectCallback(MemoryObject, virusName);
}

BOOLEAN HsScanProcessMemoryBasic(
	_In_ PHZR_SCANNER Scanner,
	_In_ HANDLE ProcessId)
{
	HS_MEMORY_PROVIDER provider;

	provider.ProcessId = ProcessId;
	provider.Callback = HsMemoryCallback;
	provider.Context = Scanner;

	return HzrVirtualQuery(&provider);
}