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

typedef struct _HS_LIST {
	ULONG Count;
	ULONG AllocationCount;

	PVOID* Items;
} HS_LIST, *PHS_LIST;

__declspec(dllexport) VOID HsInitializeList(
	_Out_ PHS_LIST List,
	_In_ ULONG InitialCapacity
	);

__declspec(dllexport) VOID HsDeleteList(
	_In_ PHS_LIST List
	);

__declspec(dllexport) VOID HsAddItemList(
	_Inout_ PHS_LIST List,
	_In_ PVOID Item
	);