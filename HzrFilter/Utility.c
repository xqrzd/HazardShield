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
#include "Context.h"
#include "Utility.h"

#define AVL_ENTRY_TAG 'vAzH'

NTSTATUS HzrFilterGetFileSize(
	_In_ PFLT_INSTANCE Instance,
	_In_ PFILE_OBJECT FileObject,
	_Out_ PLARGE_INTEGER Size)
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
		*Size = standardInfo.EndOfFile;

	return status;
}

NTSTATUS HzrFilterGetFileId64(
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

BOOLEAN HzrFilterIsPrefetchEcpPresent(
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

BOOLEAN HzrFilterIsPrefetchContextPresent(
	_In_ PFLT_INSTANCE Instance,
	_In_ PFILE_OBJECT FileObject)
{
	NTSTATUS status;
	PFILTER_STREAMHANDLE_CONTEXT context;
	BOOLEAN prefetchOpen = FALSE;

	status = FltGetStreamHandleContext(
		Instance,
		FileObject,
		&context);

	if (NT_SUCCESS(status))
	{
		prefetchOpen = context->PrefetchOpen;
		FltReleaseContext(context);
	}

	return prefetchOpen;
}

PVOID NTAPI AvlAllocate(
	_In_ PRTL_AVL_TABLE Table,
	_In_ CLONG ByteSize)
{
	UNREFERENCED_PARAMETER(Table);

	return ExAllocatePoolWithTag(PagedPool, ByteSize, AVL_ENTRY_TAG);
}

VOID NTAPI AvlFree(
	_In_ PRTL_AVL_TABLE Table,
	_In_ __drv_freesMem(Mem) _Post_invalid_ PVOID Entry)
{
	UNREFERENCED_PARAMETER(Table);

	ExFreePoolWithTag(Entry, AVL_ENTRY_TAG);
}

RTL_GENERIC_COMPARE_RESULTS AvlCompareNtfsEntry(
	_In_ PRTL_AVL_TABLE Table,
	_In_ PVOID Lhs,
	_In_ PVOID Rhs)
{
	PNTFS_CACHE_ENTRY lhs = (PNTFS_CACHE_ENTRY)Lhs;
	PNTFS_CACHE_ENTRY rhs = (PNTFS_CACHE_ENTRY)Rhs;

	UNREFERENCED_PARAMETER(Table);

	if (lhs->FileId.IndexNumber.QuadPart < rhs->FileId.IndexNumber.QuadPart)
		return GenericLessThan;
	else if (lhs->FileId.IndexNumber.QuadPart > rhs->FileId.IndexNumber.QuadPart)
		return GenericGreaterThan;
	else
		return GenericEqual;
}

VOID AvlDeleteAllElements(
	_In_ PRTL_AVL_TABLE Table)
{
	PVOID entry;

	while (!RtlIsGenericTableEmptyAvl(Table))
	{
		entry = RtlGetElementGenericTableAvl(Table, 0);
		RtlDeleteElementGenericTableAvl(Table, entry);
	}
}