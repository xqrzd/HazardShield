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

#include "HzrFilter.h"

UNICODE_STRING PortName = RTL_CONSTANT_STRING(L"\\HzrFilterPort");

NTSTATUS HspFilterPortConnect(
	_In_ PFLT_PORT ClientPort,
	_In_opt_ PVOID ServerPortCookie,
	_In_reads_bytes_opt_(SizeOfContext) PVOID ConnectionContext,
	_In_ ULONG SizeOfContext,
	_Outptr_result_maybenull_ PVOID* ConnectionCookie
	);

VOID HspFilterPortDisconnect(
	_In_opt_ PVOID ConnectionCookie
	);

NTSTATUS HspFilterClientMessage(
	_In_ PVOID PortCookie,
	_In_opt_ PVOID InputBuffer,
	_In_ ULONG InputBufferLength,
	_Out_opt_ PVOID OutputBuffer,
	_In_ ULONG OutputBufferLength,
	_Out_ PULONG ReturnOutputBufferLength
	);

NTSTATUS HspGetScanContextSynchronized(
	_In_ LONGLONG ScanId,
	_Out_ PHS_SCAN_CONTEXT* ScanContext
	);

NTSTATUS HspHandleCmdCreateSectionForDataScan(
	_Inout_ PHS_SCAN_CONTEXT ScanContext,
	_Out_ PHANDLE SectionHandle
	);

NTSTATUS HsFilterCreateCommunicationPort()
{
	NTSTATUS status;
	PSECURITY_DESCRIPTOR securityDescriptor;

	// Only allow admin or system to access the driver.

	status = FltBuildDefaultSecurityDescriptor(&securityDescriptor, FLT_PORT_ALL_ACCESS);

	if (NT_SUCCESS(status))
	{
		OBJECT_ATTRIBUTES objectAttributes;

		InitializeObjectAttributes(
			&objectAttributes,
			&PortName,
			OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE,
			NULL,
			securityDescriptor);

		// Create a communication port for the user-mode app to send commands to.

		status = FltCreateCommunicationPort(
			GlobalData.Filter,
			&GlobalData.ServerPort,
			&objectAttributes,
			NULL,
			HspFilterPortConnect,
			HspFilterPortDisconnect,
			HspFilterClientMessage,
			1);

		FltFreeSecurityDescriptor(securityDescriptor);
	}

	return status;
}

NTSTATUS HspFilterPortConnect(
	_In_ PFLT_PORT ClientPort,
	_In_opt_ PVOID ServerPortCookie,
	_In_reads_bytes_opt_(SizeOfContext) PVOID ConnectionContext,
	_In_ ULONG SizeOfContext,
	_Outptr_result_maybenull_ PVOID* ConnectionCookie)
{
	UNREFERENCED_PARAMETER(ServerPortCookie);
	UNREFERENCED_PARAMETER(ConnectionContext);
	UNREFERENCED_PARAMETER(SizeOfContext);
	UNREFERENCED_PARAMETER(ConnectionCookie);

	// Only allow SYSTEM processes to connect. Starting with Windows Vista,
	// SYSTEM processes run under a separate session.
	//if (PsGetCurrentProcessSessionId() != 0)
		//return STATUS_ACCESS_DENIED;

	GlobalData.ClientProcess = IoGetCurrentProcess();
	GlobalData.ClientPort = ClientPort;

	HsProtectProcess(GlobalData.ClientProcess, (ACCESS_MASK)-1, (ACCESS_MASK)-1);

	return STATUS_SUCCESS;
}

VOID HspFilterPortDisconnect(
	_In_opt_ PVOID ConnectionCookie)
{
	UNREFERENCED_PARAMETER(ConnectionCookie);

	// This call sets FilterData.ClientPort to NULL.
	FltCloseClientPort(GlobalData.Filter, &GlobalData.ClientPort);

	GlobalData.ClientProcess = NULL;
}

NTSTATUS HspFilterClientMessage(
	_In_ PVOID PortCookie,
	_In_opt_ PVOID InputBuffer,
	_In_ ULONG InputBufferLength,
	_Out_opt_ PVOID OutputBuffer,
	_In_ ULONG OutputBufferLength,
	_Out_ PULONG ReturnOutputBufferLength)
{
	NTSTATUS status = STATUS_INVALID_PARAMETER;
	HS_SERVICE_COMMAND request;

	UNREFERENCED_PARAMETER(PortCookie);

	// Validate input buffer size. It needs to be at least as large as the
	// command opcode (ULONG), and no greater than the largest command.

	if (InputBuffer == NULL ||
		InputBufferLength < sizeof(ULONG) ||
		InputBufferLength > sizeof(HS_SERVICE_COMMAND))
	{
		return STATUS_INVALID_PARAMETER;
	}

	// The filter manager calls ProbeForRead / ProbeForWrite on the user buffers,
	// however the user buffers must still be accessed in a try / catch to avoid
	// potential access violations.

	__try
	{
		RtlCopyMemory(&request, InputBuffer, InputBufferLength);
	}
	__except (EXCEPTION_EXECUTE_HANDLER)
	{
		return GetExceptionCode();
	}

	switch (request.Command)
	{
	case HsCmdCreateSectionForDataScan:
	{
		PHS_SCAN_CONTEXT scanContext;
		HANDLE sectionHandle;

		// Ensure there is enough space in the output buffer to
		// store the returned section handle.

		if (!OutputBuffer || OutputBufferLength < sizeof(HANDLE))
			return STATUS_INVALID_PARAMETER;

		status = HspGetScanContextSynchronized(
			request.CreateSectionForDataScan.ScanId,
			&scanContext);

		if (!NT_SUCCESS(status))
		{
			DbgPrint("Unable to find scan context! ScanId: %lld",
				request.CreateSectionForDataScan.ScanId);

			return status;
		}

		if (scanContext->SectionContext)
		{
			DbgPrint("Already created section context for scan %lld",
				request.CreateSectionForDataScan.ScanId);

			return STATUS_UNSUCCESSFUL;
		}

		status = HspHandleCmdCreateSectionForDataScan(
			scanContext,
			&sectionHandle);

		if (!NT_SUCCESS(status))
			return status;

		__try
		{
			RtlCopyMemory(OutputBuffer, &sectionHandle, sizeof(HANDLE));
			*ReturnOutputBufferLength = sizeof(HANDLE);
		}
		__except (EXCEPTION_EXECUTE_HANDLER)
		{
			// Failed to copy the section handle. The user-mode application is
			// normally responsible for closing the section handle, so it needs
			// to be manually closed here.

			// Note that HsReleaseSectionContext does not need to be called here,
			// as it will still be called in HsFilterScanFile.

			NtClose(sectionHandle);

			return GetExceptionCode();
		}

		break;
	}
	case HsCmdQueryFileName:
	{
		PHS_SCAN_CONTEXT scanContext;
		PUNICODE_STRING fileName;

		status = HspGetScanContextSynchronized(
			request.QueryFileName.ScanId,
			&scanContext);

		if (!NT_SUCCESS(status))
		{
			DbgPrint("Unable to find scan context! ScanId: %lld",
				request.QueryFileName.ScanId);

			return status;
		}

		fileName = &scanContext->FileName->Name;

		if (!OutputBuffer || OutputBufferLength < fileName->Length)
			return STATUS_BUFFER_OVERFLOW;

		__try
		{
			RtlCopyMemory(OutputBuffer, fileName->Buffer, fileName->Length);
			*ReturnOutputBufferLength = fileName->Length;
		}
		__except (EXCEPTION_EXECUTE_HANDLER)
		{
			return GetExceptionCode();
		}

		break;
	}
	}

	return status;
}

NTSTATUS HspGetScanContextSynchronized(
	_In_ LONGLONG ScanId,
	_Out_ PHS_SCAN_CONTEXT* ScanContext)
{
	NTSTATUS status = STATUS_NOT_FOUND;
	PLIST_ENTRY current;
	PLIST_ENTRY next;

	FltAcquirePushLockShared(&GlobalData.ScanContextListLock);

	LIST_FOR_EACH_SAFE(current, next, &GlobalData.ScanContextList)
	{
		PHS_SCAN_CONTEXT scanContext = CONTAINING_RECORD(current, HS_SCAN_CONTEXT, List);

		if (scanContext->ScanId == ScanId)
		{
			*ScanContext = scanContext;
			status = STATUS_SUCCESS;

			break;
		}
	}

	FltReleasePushLock(&GlobalData.ScanContextListLock);

	return status;
}

#if WINVER >= _WIN32_WINNT_WIN8
NTSTATUS HspHandleCmdCreateSectionForDataScan(
	_Inout_ PHS_SCAN_CONTEXT ScanContext,
	_Out_ PHANDLE SectionHandle)
{
	NTSTATUS status;
	PHS_SECTION_CONTEXT sectionContext;

	status = FltAllocateContext(
		GlobalData.Filter,
		FLT_SECTION_CONTEXT,
		sizeof(HS_SECTION_CONTEXT),
		PagedPool,
		&sectionContext);

	if (!NT_SUCCESS(status))
		return status;

	status = FltCreateSectionForDataScan(
		ScanContext->Instance,
		ScanContext->FileObject,
		sectionContext,
		SECTION_MAP_READ,
		NULL,
		NULL,
		PAGE_READONLY,
		SEC_COMMIT,
		0,
		&sectionContext->SectionHandle,
		&sectionContext->SectionObject,
		NULL);

	if (!NT_SUCCESS(status))
	{
		// FltCreateSectionForDataScan may fail on volumes that do
		// not support synchronization (such as network drives).
		// Fall back to FsRtlCreateSectionForDataScan.

		status = FsRtlCreateSectionForDataScan(
			&sectionContext->SectionHandle,
			&sectionContext->SectionObject,
			NULL,
			ScanContext->FileObject,
			SECTION_MAP_READ,
			NULL,
			NULL,
			PAGE_READONLY,
			SEC_COMMIT,
			0);
	}

	if (NT_SUCCESS(status))
	{
		ScanContext->SectionContext = sectionContext;
		*SectionHandle = sectionContext->SectionHandle;
	}
	else
	{
		// Failed to create a section. Since the section was never created,
		// FltReleaseContext can be used to release the section context.
		// There is no need to call FltCloseSectionForDataScan.

		FltReleaseContext(sectionContext);
	}

	return status;
}
#else
NTSTATUS HspHandleCmdCreateSectionForDataScan(
	_Inout_ PHS_SCAN_CONTEXT ScanContext,
	_Out_ PHANDLE SectionHandle)
{
	NTSTATUS status;
	PHS_SECTION_CONTEXT sectionContext;

	sectionContext = ExAllocatePoolWithTag(
		PagedPool,
		sizeof(HS_SECTION_CONTEXT),
		'nCzH');

	if (!sectionContext)
		return STATUS_INSUFFICIENT_RESOURCES;

	status = FsRtlCreateSectionForDataScan(
		&sectionContext->SectionHandle,
		&sectionContext->SectionObject,
		NULL,
		ScanContext->FileObject,
		SECTION_MAP_READ,
		NULL,
		NULL,
		PAGE_READONLY,
		SEC_COMMIT,
		0);

	if (NT_SUCCESS(status))
	{
		ScanContext->SectionContext = sectionContext;
		*SectionHandle = sectionContext->SectionHandle;
	}
	else
	{
		ExFreePoolWithTag(sectionContext, 'nCzH');
	}

	return status;
}
#endif