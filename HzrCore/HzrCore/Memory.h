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

#include <Windows.h>

typedef VOID(*PHZR_MEMORY_CALLBACK)(
	_In_ HANDLE ProcessId,
	_In_ PVOID BaseAddress,
	_In_ SIZE_T RegionSize,
	_In_opt_ PWCHAR FilePath
	);

__declspec(dllexport) BOOLEAN HzrVirtualQuery(
	_In_ HANDLE ProcessId,
	_In_ PHZR_MEMORY_CALLBACK Callback
	);