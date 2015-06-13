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

/// <summary>
/// Gets an 8-byte file reference number for the file. This number
/// is assigned by the file system and is file-system-specific.
/// https://msdn.microsoft.com/en-us/library/windows/hardware/ff540318.aspx
/// </summary>
/// <param name="Instance">Opaque instance pointer for the caller.</param>
/// <param name="FileObject">File object pointer for the file.</param>
/// <param name="FileId">An 8-byte file reference number for the file.</param>
NTSTATUS HsFilterGetFileId64(
	_In_ PFLT_INSTANCE Instance,
	_In_ PFILE_OBJECT FileObject,
	_Out_ PFILE_INTERNAL_INFORMATION FileId
	);

/// <summary>
/// Returns TRUE if the prefetcher performed the open request.
/// https://msdn.microsoft.com/en-us/library/windows/hardware/ff551843.aspx
/// </summary>
/// <param name="Filter">An opaque filter pointer to the minifilter driver.</param>
/// <param name="Data">
/// A pointer to a callback-data object of type FLT_CALLBACK_DATA,
/// which represents the create operation.
/// </param>
BOOLEAN HsFilterIsPrefetchEcpPresent(
	_In_ PFLT_FILTER Filter,
	_In_ PFLT_CALLBACK_DATA Data
	);

/// <summary>
/// Returns TRUE if the given file was opened by the prefetcher.
/// </summary>
/// <param name="Instance">Opaque instance pointer for the caller.</param>
/// <param name="FileObject">File object pointer for the file.</param>
BOOLEAN HsFilterIsPrefetchContextPresent(
	_In_ PFLT_INSTANCE Instance,
	_In_ PFILE_OBJECT FileObject
	);

PVOID HsAvlAllocate(
	_In_ PRTL_AVL_TABLE Table,
	_In_ CLONG ByteSize
	);

VOID HsAvlFree(
	_In_ PRTL_AVL_TABLE Table,
	_In_ __drv_freesMem(Mem) _Post_invalid_ PVOID Buffer
	);

RTL_GENERIC_COMPARE_RESULTS HsAvlCompareNtfsEntry(
	_In_ PRTL_AVL_TABLE Table,
	_In_ PVOID Lhs,
	_In_ PVOID Rhs
	);

/// <summary>
/// Deletes all elements in the given AVL tree.
/// </summary>
/// <param name="Table">The table to clear.</param>
VOID HsAvlDeleteAllElements(
	_In_ PRTL_AVL_TABLE Table
	);

#endif