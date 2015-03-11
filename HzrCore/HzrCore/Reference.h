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

typedef VOID(*PHS_TYPE_DELETE_PROCEDURE)(
	_In_ PVOID Object
	);

typedef struct _HS_OBJECT_TYPE {
	PHS_TYPE_DELETE_PROCEDURE DeleteProcedure;
	PSTR Name;
	ULONG NumberOfObjects;
} HS_OBJECT_TYPE, *PHS_OBJECT_TYPE;

typedef struct _HS_OBJECT_HEADER {
	LONG ReferenceCount;
	PHS_OBJECT_TYPE Type;
} HS_OBJECT_HEADER, *PHS_OBJECT_HEADER;