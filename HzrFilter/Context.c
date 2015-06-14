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

#include "Context.h"
#include "Utility.h"

#define HS_CTX_INSTANCE_TAG 'iCzH'
#define HS_CTX_STREAM_TAG 'sCzH'
#define HS_CTX_STREAMHANDLE_TAG 'hCzH'
#define HS_CTX_SECTION_TAG 'nCzH'

VOID HspInstanceContextCleanup(
	_In_ PFLT_CONTEXT Context,
	_In_ FLT_CONTEXT_TYPE ContextType
	);

VOID HspStreamContextCleanup(
	_In_ PFLT_CONTEXT Context,
	_In_ FLT_CONTEXT_TYPE ContextType
	);

VOID HspSectionContextCleanup(
	_In_ PFLT_CONTEXT Context,
	_In_ FLT_CONTEXT_TYPE ContextType
	);

const FLT_CONTEXT_REGISTRATION ContextRegistration[] = {
	{
		FLT_INSTANCE_CONTEXT,
		0,
		HspInstanceContextCleanup,
		sizeof(HS_INSTANCE_CONTEXT),
		HS_CTX_INSTANCE_TAG
	},
	{
		FLT_STREAM_CONTEXT,
		0,
		HspStreamContextCleanup,
		sizeof(HS_STREAM_CONTEXT),
		HS_CTX_STREAM_TAG
	},
	{
		FLT_STREAMHANDLE_CONTEXT,
		0,
		NULL,
		sizeof(HS_STREAMHANDLE_CONTEXT),
		HS_CTX_STREAMHANDLE_TAG
	},
	{
		FLT_SECTION_CONTEXT,
		0,
		HspSectionContextCleanup,
		sizeof(HS_SECTION_CONTEXT),
		HS_CTX_SECTION_TAG
	},

	{ FLT_CONTEXT_END }
};

VOID HspInstanceContextCleanup(
	_In_ PFLT_CONTEXT Context,
	_In_ FLT_CONTEXT_TYPE ContextType)
{
	PHS_INSTANCE_CONTEXT context = Context;

	UNREFERENCED_PARAMETER(ContextType);

	if (context->CacheSupported)
	{
		FltAcquirePushLockExclusive(&context->CacheLock);
		HsAvlDeleteAllElements(&context->CacheTable);
		FltReleasePushLock(&context->CacheLock);
		FltDeletePushLock(&context->CacheLock);
	}
}

VOID HspStreamContextCleanup(
	_In_ PFLT_CONTEXT Context,
	_In_ FLT_CONTEXT_TYPE ContextType)
{
	PHS_STREAM_CONTEXT context = Context;

	UNREFERENCED_PARAMETER(ContextType);

	FltDeletePushLock(&context->ScanLock);
}

VOID HspSectionContextCleanup(
	_In_ PFLT_CONTEXT Context,
	_In_ FLT_CONTEXT_TYPE ContextType)
{
	PHS_SECTION_CONTEXT sectionContext = Context;

	UNREFERENCED_PARAMETER(ContextType);

	ObDereferenceObject(sectionContext->SectionObject);
}

/// <summary>
/// Returns TRUE if the given file is owned by the prefetcher.
/// </summary>
/// <param name="Instance">Opaque instance pointer for the caller.</param>
/// <param name="FileObject">File object pointer for the file.</param>
BOOLEAN HsIsPrefetchContextPresent(
	_In_ PFLT_INSTANCE Instance,
	_In_ PFILE_OBJECT FileObject)
{
	NTSTATUS status;
	PHS_STREAMHANDLE_CONTEXT context;
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

/// <summary>
/// Releases a section context. Note that this does
/// not close the section handle.
/// </summary>
/// <param name="SectionContext">Pointer to the context.</param>
NTSTATUS HsReleaseSectionContext(
	_In_ PHS_SECTION_CONTEXT SectionContext)
{
	NTSTATUS status;

	status = FltCloseSectionForDataScan(SectionContext);

	if (!NT_SUCCESS(status))
		DbgPrint("FltCloseSectionForDataScan failed %X", status);

	// The user-mode application is responsible for closing SectionHandle.
	// The section cleanup routine will dereference SectionObject.

	FltReleaseContext(SectionContext);

	return status;
}