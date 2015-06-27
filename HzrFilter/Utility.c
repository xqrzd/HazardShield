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

#include <initguid.h>
#include "Utility.h"

#define HS_AVL_ENTRY_TAG 'vAzH'

/// <summary>
/// Gets an 8-byte file reference number for the file. This number
/// is assigned by the file system and is file-system-specific.
/// https://msdn.microsoft.com/en-us/library/windows/hardware/ff540318.aspx
/// </summary>
/// <param name="Instance">Opaque instance pointer for the caller.</param>
/// <param name="FileObject">File object pointer for the file.</param>
/// <param name="FileId">An 8-byte file reference number for the file.</param>
NTSTATUS HsGetFileId64(
	_In_ PFLT_INSTANCE Instance,
	_In_ PFILE_OBJECT FileObject,
	_Out_ PFILE_INTERNAL_INFORMATION FileId)
{
	return FltQueryInformationFile(
		Instance,
		FileObject,
		FileId,
		sizeof(FILE_INTERNAL_INFORMATION),
		FileInternalInformation,
		NULL);
}

/// <summary>
/// Gets the size of a file in bytes.
/// </summary>
/// <param name="Instance">Opaque instance pointer for the caller.</param>
/// <param name="FileObject">File object pointer for the file.</param>
/// <param name="FileSize">The size of the file in bytes.</param>
NTSTATUS HsGetFileSize(
	_In_ PFLT_INSTANCE Instance,
	_In_ PFILE_OBJECT FileObject,
	_Out_ PLONGLONG FileSize)
{
	NTSTATUS status;
	FILE_STANDARD_INFORMATION standardInfo;

	status = FltQueryInformationFile(
		Instance,
		FileObject,
		&standardInfo,
		sizeof(FILE_STANDARD_INFORMATION),
		FileStandardInformation,
		NULL);

	if (NT_SUCCESS(status))
		*FileSize = standardInfo.EndOfFile.QuadPart;

	return status;
}

/// <summary>
/// Returns TRUE if the prefetcher performed the open request.
/// https://msdn.microsoft.com/en-us/library/windows/hardware/ff551843.aspx
/// </summary>
/// <param name="Filter">An opaque filter pointer to the minifilter driver.</param>
/// <param name="Data">
/// A pointer to a callback-data object of type FLT_CALLBACK_DATA,
/// which represents the create operation.
/// </param>
BOOLEAN HsIsPrefetchEcpPresent(
	_In_ PFLT_FILTER Filter,
	_In_ PFLT_CALLBACK_DATA Data)
{
	NTSTATUS status;
	PECP_LIST ecpList;
	PVOID ecpContext;

	status = FltGetEcpListFromCallbackData(Filter, Data, &ecpList);

	if (NT_SUCCESS(status) && (ecpList != NULL))
	{
		status = FltFindExtraCreateParameter(
			Filter,
			ecpList,
			&GUID_ECP_PREFETCH_OPEN,
			&ecpContext,
			NULL);

		if (NT_SUCCESS(status))
		{
			if (!FltIsEcpFromUserMode(Filter, ecpContext))
				return TRUE;
		}
	}

	return FALSE;
}

PVOID HsAvlAllocate(
	_In_ PRTL_AVL_TABLE Table,
	_In_ CLONG ByteSize)
{
	UNREFERENCED_PARAMETER(Table);

	return ExAllocatePoolWithTag(PagedPool, ByteSize, HS_AVL_ENTRY_TAG);
}

VOID HsAvlFree(
	_In_ PRTL_AVL_TABLE Table,
	_In_ __drv_freesMem(Mem) _Post_invalid_ PVOID Buffer)
{
	UNREFERENCED_PARAMETER(Table);

	ExFreePoolWithTag(Buffer, HS_AVL_ENTRY_TAG);
}

RTL_GENERIC_COMPARE_RESULTS HsAvlCompareNtfsEntry(
	_In_ PRTL_AVL_TABLE Table,
	_In_ PVOID Lhs,
	_In_ PVOID Rhs)
{
	PHS_NTFS_CACHE_ENTRY lhs = Lhs;
	PHS_NTFS_CACHE_ENTRY rhs = Rhs;

	UNREFERENCED_PARAMETER(Table);

	if (lhs->FileId.IndexNumber.QuadPart < rhs->FileId.IndexNumber.QuadPart)
		return GenericLessThan;
	else if (lhs->FileId.IndexNumber.QuadPart > rhs->FileId.IndexNumber.QuadPart)
		return GenericGreaterThan;
	else
		return GenericEqual;
}

/// <summary>
/// Deletes all elements in the given AVL tree.
/// </summary>
/// <param name="Table">The table to clear.</param>
VOID HsAvlDeleteAllElements(
	_In_ PRTL_AVL_TABLE Table)
{
	PVOID entry;

	while (!RtlIsGenericTableEmptyAvl(Table))
	{
		entry = RtlGetElementGenericTableAvl(Table, 0);
		RtlDeleteElementGenericTableAvl(Table, entry);
	}
}