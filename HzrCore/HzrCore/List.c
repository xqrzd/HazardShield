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

#include "List.h"

VOID HsInitializeList(
	_Out_ PHS_LIST List,
	_In_ ULONG InitialCapacity)
{
	if (InitialCapacity == 0)
		InitialCapacity = 1;

	List->Count = 0;
	List->AllocationCount = InitialCapacity;

	List->Items = HsAllocate(InitialCapacity * sizeof(PVOID));
}

VOID HsClearList(
	_Inout_ PHS_LIST List)
{
	List->Count = 0;
}

VOID HsDeleteList(
	_In_ PHS_LIST List)
{
	HsFree(List->Items);
}

VOID HsAddItemList(
	_Inout_ PHS_LIST List,
	_In_ PVOID Item)
{
	if (List->Count == List->AllocationCount)
	{
		// Need to increase array size.

		List->AllocationCount *= 2;
		List->Items = HsReAllocate(List->Items, List->AllocationCount * sizeof(PVOID));
	}

	List->Items[List->Count] = Item;
	List->Count++;
}