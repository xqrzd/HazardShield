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
#include "String2.h"

HS_OBJECT_TYPE HsStringType;

VOID HsInitializeStringType()
{
	HsInitializeObjectType(
		&HsStringType,
		NULL);
}

PHS_STRING HsCreateString(
	_In_ PWSTR Buffer)
{
	return HsCreateStringEx(Buffer, wcslen(Buffer));
}

PHS_STRING HsCreateStringEx(
	_In_opt_ PWCHAR Buffer,
	_In_ SIZE_T Length)
{
	PHS_STRING string;
	SIZE_T size;

	size = sizeof(HS_STRING) + (Length * sizeof(WCHAR));

	string = HsCreateObject(
		size,
		&HsStringType);

	RtlCopyMemory(
		&string->Buffer,
		Buffer,
		Length * sizeof(WCHAR));

	// Set null terminator.
	string->Buffer[Length] = L'\0';

	string->Length = Length;

	return string;
}