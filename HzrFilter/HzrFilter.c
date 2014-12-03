/*
*  Copyright (C) 2014 Orbitech
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

struct {
	PFLT_FILTER Filter;

	PFLT_PORT ServerPort;
	PFLT_PORT ClientPort;
	PEPROCESS ClientProcess;

	HANDLE_SYSTEM HandleSystem;
} FilterData;

CONST FLT_OPERATION_REGISTRATION Callbacks[] = {
	{
		IRP_MJ_ACQUIRE_FOR_SECTION_SYNCHRONIZATION,
		0,
		HzrFilterPreAcquireForSectionSynchronization,
		NULL
	},

	{ IRP_MJ_OPERATION_END }
};

CONST FLT_REGISTRATION FilterRegistration = {
	sizeof(FLT_REGISTRATION),	// Size
	FLT_REGISTRATION_VERSION,	// Version
	0,							// Flags

	ContextRegistration,		// Context
	Callbacks,					// Operation callbacks

	HzrFilterUnload,			// MiniFilterUnload

	HzrFilterInstanceSetup,				// InstanceSetup
	HzrFilterInstanceQueryTeardown,		// InstanceQueryTeardown
	NULL,								// InstanceTeardownStart
	NULL,								// InstanceTeardownComplete

	NULL,	// GenerateFileName
	NULL,	// GenerateDestinationFileName
	NULL	// NormalizeNameComponent
};

NTSTATUS HzrFilterInstanceSetup(
	_In_ PCFLT_RELATED_OBJECTS FltObjects,
	_In_ FLT_INSTANCE_SETUP_FLAGS Flags,
	_In_ DEVICE_TYPE VolumeDeviceType,
	_In_ FLT_FILESYSTEM_TYPE VolumeFilesystemType)
{
	NTSTATUS status;
	PFILTER_INSTANCE_CONTEXT context;

	UNREFERENCED_PARAMETER(Flags);

	status = FltAllocateContext(
		FltObjects->Filter,
		FLT_INSTANCE_CONTEXT,
		sizeof(FILTER_INSTANCE_CONTEXT),
		PagedPool,
		&context);

	if (NT_SUCCESS(status))
	{
		status = FltSetInstanceContext(
			FltObjects->Instance,
			FLT_SET_CONTEXT_KEEP_IF_EXISTS,
			context,
			NULL);

		if (NT_SUCCESS(status))
		{
			if (VolumeDeviceType == FILE_DEVICE_DISK_FILE_SYSTEM &&
				VolumeFilesystemType == FLT_FSTYPE_NTFS)
			{
				context->CacheSupported = TRUE;
			}
			else
				context->CacheSupported = FALSE;
		}
		else
			DbgPrint("HzrFilterInstanceSetup::FltSetInstanceContext failed %X", status);

		// Always release the context.
		FltReleaseContext(context);
	}
	else
		DbgPrint("HzrFilterInstanceSetup::FltAllocateContext failed %X", status);

	// Only attach to the volume if the instance context was set.
	return NT_SUCCESS(status) ? STATUS_SUCCESS : STATUS_FLT_DO_NOT_ATTACH;
}

NTSTATUS HzrFilterInstanceQueryTeardown(
	_In_ PCFLT_RELATED_OBJECTS FltObjects,
	_In_ FLT_INSTANCE_QUERY_TEARDOWN_FLAGS Flags)
{
	UNREFERENCED_PARAMETER(FltObjects);
	UNREFERENCED_PARAMETER(Flags);

	return STATUS_SUCCESS;
}

NTSTATUS DriverEntry(
	_In_ PDRIVER_OBJECT DriverObject,
	_In_ PUNICODE_STRING RegistryPath)
{
	NTSTATUS status;

	UNREFERENCED_PARAMETER(RegistryPath);

	status = FltRegisterFilter(DriverObject, &FilterRegistration, &FilterData.Filter);

	if (NT_SUCCESS(status))
	{
		PSECURITY_DESCRIPTOR securityDescriptor;

		// Only allow admins or system to access the driver.
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

			// Create communication port so the user-mode app can access the driver.
			status = FltCreateCommunicationPort(
				FilterData.Filter,
				&FilterData.ServerPort,
				&objectAttributes,
				NULL,
				HzrFilterPortConnect,
				HzrFilterPortDisconnect,
				HzrFilterClientMessage,
				1);

			FltFreeSecurityDescriptor(securityDescriptor);

			if (NT_SUCCESS(status))
			{
				status = FltStartFiltering(FilterData.Filter);

				if (NT_SUCCESS(status))
				{
					// Init
					DbgPrint("HzrFilter loaded successfully");
				}
			}

			if (!NT_SUCCESS(status))
				FltCloseCommunicationPort(FilterData.ServerPort);
		}

		if (!NT_SUCCESS(status))
			FltUnregisterFilter(FilterData.Filter);
	}

	return status;
}

NTSTATUS HzrFilterUnload(
	_In_ FLT_FILTER_UNLOAD_FLAGS Flags)
{
	UNREFERENCED_PARAMETER(Flags);

	FltCloseCommunicationPort(FilterData.ServerPort);
	FltUnregisterFilter(FilterData.Filter);

	return STATUS_SUCCESS;
}

NTSTATUS HzrFilterPortConnect(
	_In_ PFLT_PORT ClientPort,
	_In_opt_ PVOID ServerPortCookie,
	_In_reads_bytes_opt_(SizeOfContext) PVOID ConnectionContext,
	_In_ ULONG SizeOfContext,
	_Outptr_result_maybenull_ PVOID *ConnectionCookie)
{
	UNREFERENCED_PARAMETER(ServerPortCookie);
	UNREFERENCED_PARAMETER(ConnectionContext);
	UNREFERENCED_PARAMETER(SizeOfContext);
	UNREFERENCED_PARAMETER(ConnectionCookie);

	FilterData.ClientProcess = IoGetCurrentProcess();
	FilterData.ClientPort = ClientPort;

	DbgPrint("Client connected %d", PsGetCurrentProcessId());

	return STATUS_SUCCESS;
}

VOID HzrFilterPortDisconnect(
	_In_opt_ PVOID ConnectionCookie)
{
	UNREFERENCED_PARAMETER(ConnectionCookie);

	// This call sets FilterData.ClientPort to NULL
	FltCloseClientPort(FilterData.Filter, &FilterData.ClientPort);

	FilterData.ClientProcess = NULL;

	DbgPrint("Client disconnected");
}

NTSTATUS HzrFilterClientMessage(
	_In_ PVOID PortCookie,
	_In_opt_ PVOID InputBuffer,
	_In_ ULONG InputBufferLength,
	_Out_opt_ PVOID OutputBuffer,
	_In_ ULONG OutputBufferLength,
	_Out_ PULONG ReturnOutputBufferLength)
{
	NTSTATUS status = STATUS_INVALID_PARAMETER;
	UCHAR capturedInput[sizeof(PVOID) * 4];
	ULONG command;

	UNREFERENCED_PARAMETER(PortCookie);

	if (InputBuffer == NULL || InputBufferLength == 0 || InputBufferLength > sizeof(capturedInput))
		return STATUS_INVALID_PARAMETER;

	// The filter manager calls ProbeForRead/Write on the user buffers, however
	// they must still be accessed in a try/except, to avoid potential access violations.
	__try
	{
		RtlCopyMemory(capturedInput, InputBuffer, InputBufferLength);
	}
	__except (EXCEPTION_EXECUTE_HANDLER)
	{
		return GetExceptionCode();
	}

	command = *(PULONG)capturedInput;

	if (command == DRV_CMD_GET_BUFFER)
	{
		PSERVICE_REQUEST_BUFFER request = (PSERVICE_REQUEST_BUFFER)capturedInput;
		PBUFFER_INFO bufferInfo;

		// Lookup the buffer the client is requesting.
		status = HndLookupObject(&FilterData.HandleSystem, request->Handle, &bufferInfo);
		if (NT_SUCCESS(status))
		{
			if (bufferInfo->BufferSize > OutputBufferLength)
				return STATUS_INVALID_PARAMETER;

			__try
			{
				RtlCopyMemory(OutputBuffer, bufferInfo->Buffer, bufferInfo->BufferSize);
			}
			__except (EXCEPTION_EXECUTE_HANDLER)
			{
				return GetExceptionCode();
			}

			*ReturnOutputBufferLength = bufferInfo->BufferSize;
		}
	}

	return status;
}

FLT_PREOP_CALLBACK_STATUS HzrFilterPreAcquireForSectionSynchronization(
	_Inout_ PFLT_CALLBACK_DATA Data,
	_In_ PCFLT_RELATED_OBJECTS FltObjects,
	_Flt_CompletionContext_Outptr_ PVOID *CompletionContext)
{
	UNREFERENCED_PARAMETER(FltObjects);
	UNREFERENCED_PARAMETER(CompletionContext);
	UNREFERENCED_PARAMETER(Data);

	if (FlagOn(Data->Iopb->Parameters.AcquireForSectionSynchronization.PageProtection, PAGE_EXECUTE) &&
		Data->Iopb->Parameters.AcquireForSectionSynchronization.SyncType == SyncTypeCreateSection)
	{
	}

	// Pass the I/O operation through without calling the minifilter's postoperation callback on completion.
	return FLT_PREOP_SUCCESS_NO_CALLBACK;
}

NTSTATUS HzrFilterScanFile(
	_In_ PFLT_INSTANCE Instance,
	_In_ PFILE_OBJECT FileObject,
	_In_ UCHAR FileAccess,
	_Out_ PSERVICE_RESPONSE Response)
{
	NTSTATUS status;
	LARGE_INTEGER fileSize;

	status = HzrFilterGetFileSize(Instance, FileObject, &fileSize);
	if (NT_SUCCESS(status))
	{
		if (fileSize.QuadPart <= MAX_FILE_SCAN_SIZE)
		{
			POBJECT_NAME_INFORMATION fullFilePath;

			status = IoQueryFileDosDeviceName(FileObject, &fullFilePath);
			if (NT_SUCCESS(status))
			{
				PVOID buffer;

				buffer = ExAllocatePoolWithTag(PagedPool, fileSize.LowPart, 'file');
				if (buffer)
				{
					LARGE_INTEGER byteOffset;

					byteOffset.QuadPart = 0;

					status = FltReadFile(
						Instance,
						FileObject,
						&byteOffset,
						fileSize.LowPart,
						buffer,
						0, NULL, NULL, NULL);

					if (NT_SUCCESS(status))
					{
						status = SvcScanFile(
							&FilterData.HandleSystem,
							FilterData.Filter,
							&FilterData.ClientPort,
							FileAccess,
							fullFilePath,
							buffer,
							fileSize.LowPart,
							Response);
					}
					else
						DbgPrint("HzrFilterScanFile: FltReadFile failed %X", status);

					ExFreePoolWithTag(buffer, 'file');
				}
				else
					status = STATUS_INSUFFICIENT_RESOURCES;

				ExFreePool(fullFilePath);
			}
			else
				DbgPrint("HzrFilterScanFile: IoQueryFileDosDeviceName failed %X", status);
		}
		else
		{
			status = STATUS_FILE_TOO_LARGE;
			DbgPrint("Ignoring large file");
		}
	}
	else
		DbgPrint("HzrFilterScanFile: HzrFilterGetFileSize failed %X", status);

	return status;
}