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

#include "Scanner.h"

cl_error_t HzrInitClamAv()
{
	return cl_init(CL_INIT_DEFAULT);
}

BOOLEAN HzrInitScanner(
	_Out_ PHZR_SCANNER Scanner)
{
	Scanner->Engine = cl_engine_new();

	if (Scanner->Engine)
	{
		Scanner->Signatures = 0;

		return TRUE;
	}

	return FALSE;
}

VOID HzrFreeScanner(
	_In_ PHZR_SCANNER Scanner)
{
	cl_engine_free(Scanner->Engine);
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