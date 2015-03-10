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

#ifdef _DEBUG
#include <malloc.h>
#endif

#ifndef FlagOn
#define FlagOn(_F,_SF)        ((_F) & (_SF))
#endif

#ifndef FlagOff
#define FlagOff(_F,_SF)        (!((_F) & (_SF)))
#endif

PVOID FORCEINLINE HsAllocate(
	_In_ SIZE_T Size)
{
#ifdef _DEBUG
	return malloc(Size);
#else
	return HeapAlloc(GetProcessHeap(), HEAP_GENERATE_EXCEPTIONS, Size);
#endif
}

PVOID FORCEINLINE HsReAllocate(
	_In_ PVOID Memory,
	_In_ SIZE_T Size)
{
#ifdef _DEBUG
	return realloc(Memory, Size);
#else
	return HeapReAlloc(GetProcessHeap(), HEAP_GENERATE_EXCEPTIONS, Memory, Size);
#endif
}

VOID FORCEINLINE HsFree(
	_In_ PVOID Buffer)
{
#ifdef _DEBUG
	free(Buffer);
#else
	HeapFree(GetProcessHeap(), 0, Buffer);
#endif
}