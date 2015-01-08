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

#ifndef HZRFILTER_H
#define HZRFILTER_H

#include <fltKernel.h>
#include "Context.h"
#include "Handle.h"
#include "Protect.h"
#include "Service.h"
#include "Utility.h"

#define MAX_FILE_SCAN_SIZE 8388608 // 8 MB

DRIVER_INITIALIZE DriverEntry;
NTSTATUS DriverEntry(
	_In_ PDRIVER_OBJECT DriverObject,
	_In_ PUNICODE_STRING RegistryPath
	);

NTSTATUS HzrFilterInstanceSetup(
	_In_ PCFLT_RELATED_OBJECTS FltObjects,
	_In_ FLT_INSTANCE_SETUP_FLAGS Flags,
	_In_ DEVICE_TYPE VolumeDeviceType,
	_In_ FLT_FILESYSTEM_TYPE VolumeFilesystemType
	);

NTSTATUS HzrFilterUnload(
	_In_ FLT_FILTER_UNLOAD_FLAGS Flags
	);

NTSTATUS HzrFilterPortConnect(
	_In_ PFLT_PORT ClientPort,
	_In_opt_ PVOID ServerPortCookie,
	_In_reads_bytes_opt_(SizeOfContext) PVOID ConnectionContext,
	_In_ ULONG SizeOfContext,
	_Outptr_result_maybenull_ PVOID *ConnectionCookie
	);

VOID HzrFilterPortDisconnect(
	_In_opt_ PVOID ConnectionCookie
	);

NTSTATUS HzrFilterClientMessage(
	_In_ PVOID PortCookie,
	_In_opt_ PVOID InputBuffer,
	_In_ ULONG InputBufferLength,
	_Out_opt_ PVOID OutputBuffer,
	_In_ ULONG OutputBufferLength,
	_Out_ PULONG ReturnOutputBufferLength
	);

NTSTATUS HzrFilterInstanceQueryTeardown(
	_In_ PCFLT_RELATED_OBJECTS FltObjects,
	_In_ FLT_INSTANCE_QUERY_TEARDOWN_FLAGS Flags
	);

FLT_POSTOP_CALLBACK_STATUS HzrFilterPostCreate(
	_Inout_ PFLT_CALLBACK_DATA Data,
	_In_ PCFLT_RELATED_OBJECTS FltObjects,
	_In_opt_ PVOID CompletionContext,
	_In_ FLT_POST_OPERATION_FLAGS Flags
	);

FLT_PREOP_CALLBACK_STATUS HzrFilterPreWrite(
	_Inout_ PFLT_CALLBACK_DATA Data,
	_In_ PCFLT_RELATED_OBJECTS FltObjects,
	_Flt_CompletionContext_Outptr_ PVOID *CompletionContext
	);

FLT_PREOP_CALLBACK_STATUS HzrFilterPreSetInformation(
	_Inout_ PFLT_CALLBACK_DATA Data,
	_In_ PCFLT_RELATED_OBJECTS FltObjects,
	_Flt_CompletionContext_Outptr_ PVOID *CompletionContext
	);

FLT_PREOP_CALLBACK_STATUS HzrFilterPreAcquireForSectionSynchronization(
	_Inout_ PFLT_CALLBACK_DATA Data,
	_In_ PCFLT_RELATED_OBJECTS FltObjects,
	_Flt_CompletionContext_Outptr_ PVOID *CompletionContext
	);

FLT_PREOP_CALLBACK_STATUS HzrFilterPreCleanup(
	_Inout_ PFLT_CALLBACK_DATA Data,
	_In_ PCFLT_RELATED_OBJECTS FltObjects,
	_Flt_CompletionContext_Outptr_ PVOID *CompletionContext
	);

NTSTATUS HzrFilterScanStream(
	_In_ PFLT_INSTANCE Instance,
	_In_ PFILE_OBJECT FileObject,
	_In_ UCHAR FileAccess,
	_Out_ PSERVICE_RESPONSE Response
	);

NTSTATUS HzrFilterScanFile(
	_In_ PFLT_INSTANCE Instance,
	_In_ PFILE_OBJECT FileObject,
	_In_ UCHAR FileAccess,
	_Out_ PSERVICE_RESPONSE Response
	);

NTSTATUS HzrFilterDeleteFile(
	_In_ PFLT_INSTANCE Instance,
	_In_ PFILE_OBJECT FileObject
	);

NTSTATUS HzrFilterMarkStreamModified(
	_In_ PFLT_INSTANCE Instance,
	_In_ PFILE_OBJECT FileObject
	);

NTSTATUS HzrFilterGetFileCacheStatus(
	_In_ PFLT_INSTANCE Instance,
	_In_ PFILE_OBJECT FileObject,
	_Out_ PBOOLEAN Infected
	);

NTSTATUS HzrFilterSyncCache(
	_In_ PFLT_INSTANCE Instance,
	_In_ PFILE_OBJECT FileObject,
	_In_ PFILTER_STREAM_CONTEXT StreamContext
	);

#endif