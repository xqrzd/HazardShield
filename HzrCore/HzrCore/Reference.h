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

/// <summary>
/// Gets a pointer to an object from an object header.
/// </summary>
/// <param name="ObjectHeader">A pointer to an object header.</param>
/// <returns>A pointer to the object.</returns>
#define HsObjectHeaderToObject(ObjectHeader) (PTR_ADD_OFFSET((ObjectHeader), (sizeof(HS_OBJECT_HEADER))))

/// <summary>
/// Gets a pointer an object header from an object.
/// </summary>
/// <param name="Object">A pointer to an object.</param>
/// <returns>A pointer to the object's header.</returns>
#define HsObjectToObjectHeader(Object) (PTR_SUB_OFFSET((Object), (sizeof(HS_OBJECT_HEADER))))

typedef VOID(*PHS_TYPE_DELETE_PROCEDURE)(
	_In_ PVOID Object
	);

typedef struct _HS_OBJECT_TYPE {
	/// <summary>
	/// An optional function called when objects of this type are freed.
	/// </summary>
	PHS_TYPE_DELETE_PROCEDURE DeleteProcedure;
} HS_OBJECT_TYPE, *PHS_OBJECT_TYPE;

typedef struct _HS_OBJECT_HEADER {
	/// <summary>
	/// The reference count for this object.
	/// </summary>
	LONG ReferenceCount;

	/// <summary>
	/// The type of object.
	/// </summary>
	PHS_OBJECT_TYPE Type;
} HS_OBJECT_HEADER, *PHS_OBJECT_HEADER;

/// <summary>
/// Creates an object managed by the reference counting system.
/// </summary>
/// <param name="ObjectSize">The size of the object.</param>
/// <param name="ObjectType">The type of object.</param>
__declspec(dllexport) PVOID HsCreateObject(
	_In_ SIZE_T ObjectSize,
	_In_ PHS_OBJECT_TYPE ObjectType
	);

/// <summary>
/// Initializes an object type structure. This must be called for every
/// object type used in the reference counting system.
/// </summary>
/// <param name="ObjectType">A pointer to an object type.</param>
/// <param name="DeleteProcedure">An optional delete procedure, which is be called when an object of this type is deleted.</param>
__declspec(dllexport) VOID HsInitializeObjectType(
	_Out_ PHS_OBJECT_TYPE ObjectType,
	_In_opt_ PHS_TYPE_DELETE_PROCEDURE DeleteProcedure
	);

/// <summary>
/// Increments an object's reference count by 1.
/// </summary>
/// <param name="Object">A pointer to an object.</param>
__declspec(dllexport) VOID HsReferenceObject(
	_In_ PVOID Object
	);

/// <summary>
/// Decrements an object's reference count by 1.
/// </summary>
/// <param name="Object">A pointer to an object.</param>
__declspec(dllexport) VOID HsDereferenceObject(
	_In_ PVOID Object
	);