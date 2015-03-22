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

#include "Reference.h"

VOID HspFreeObject(
	_In_ PHS_OBJECT_HEADER ObjectHeader
	);

PVOID HsCreateObject(
	_In_ SIZE_T ObjectSize,
	_In_ PHS_OBJECT_TYPE ObjectType)
{
	PHS_OBJECT_HEADER objectHeader;
	SIZE_T objectSize;

	// Allocate memory for the object and its header.
	objectSize = sizeof(HS_OBJECT_HEADER) + ObjectSize;
	objectHeader = HsAllocate(objectSize);

	objectHeader->ReferenceCount = 1;
	objectHeader->Type = ObjectType;

	return HsObjectHeaderToObject(objectHeader);
}

VOID HsInitializeObjectType(
	_Out_ PHS_OBJECT_TYPE ObjectType,
	_In_opt_ PHS_TYPE_DELETE_PROCEDURE DeleteProcedure)
{
	ObjectType->DeleteProcedure = DeleteProcedure;
}

VOID HsReferenceObject(
	_In_ PVOID Object)
{
	PHS_OBJECT_HEADER objectHeader;

	objectHeader = HsObjectToObjectHeader(Object);

	InterlockedIncrement(&objectHeader->ReferenceCount);
}

VOID HsDereferenceObject(
	_In_ PVOID Object)
{
	PHS_OBJECT_HEADER objectHeader;
	LONG newRefCount;

	objectHeader = HsObjectToObjectHeader(Object);

	newRefCount = InterlockedDecrement(&objectHeader->ReferenceCount);

	if (newRefCount == 0)
		HspFreeObject(objectHeader);
}

VOID HspFreeObject(
	_In_ PHS_OBJECT_HEADER ObjectHeader)
{
	// Call the object type specific delete procedure, if one exists.
	if (ObjectHeader->Type->DeleteProcedure)
	{
		ObjectHeader->Type->DeleteProcedure(
			HsObjectHeaderToObject(ObjectHeader));
	}

	HsFree(ObjectHeader);
}