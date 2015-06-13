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

#ifndef HZRFILTER_UTILITY_H
#define HZRFILTER_UTILITY_H

#include <fltKernel.h>

#define LIST_FOR_EACH_SAFE(Current, Next, Head) for (Current = (Head)->Flink, Next = Current->Flink; Current != (Head); Current = Next, Next = Current->Flink)

typedef struct _NTFS_CACHE_ENTRY {
	FILE_INTERNAL_INFORMATION FileId;
	BOOLEAN Infected;
} NTFS_CACHE_ENTRY, *PNTFS_CACHE_ENTRY;

NTSTATUS HzrFilterGetFileId64(
	_In_ PFLT_INSTANCE Instance,
	_In_ PFILE_OBJECT FileObject,
	_Out_ PFILE_INTERNAL_INFORMATION FileId
	);

BOOLEAN HzrFilterIsPrefetchEcpPresent(
	_In_ PFLT_FILTER Filter,
	_In_ PFLT_CALLBACK_DATA Data
	);

BOOLEAN HzrFilterIsPrefetchContextPresent(
	_In_ PFLT_INSTANCE Instance,
	_In_ PFILE_OBJECT FileObject
	);

PVOID NTAPI AvlAllocate(
	_In_ PRTL_AVL_TABLE Table,
	_In_ CLONG ByteSize
	);

VOID NTAPI AvlFree(
	_In_ PRTL_AVL_TABLE Table,
	_In_ __drv_freesMem(Mem) _Post_invalid_ PVOID Entry
	);

RTL_GENERIC_COMPARE_RESULTS AvlCompareNtfsEntry(
	_In_ PRTL_AVL_TABLE Table,
	_In_ PVOID Lhs,
	_In_ PVOID Rhs
	);

VOID AvlDeleteAllElements(
	_In_ PRTL_AVL_TABLE Table
	);

#endif