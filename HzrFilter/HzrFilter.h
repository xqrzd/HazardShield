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

// Warning C4201: nonstandard extension used : nameless struct/union
#pragma warning (disable: 4201)

#include <fltKernel.h>
#include "Context.h"
#include "Protect.h"
#include "Utility.h"

#define HS_MIN_FILE_SCAN_SIZE 64
#define HS_MAX_FILE_SCAN_SIZE 52428800 // 50 MB

#define HS_RESPONSE_FLAG_INFECTED 0x1
#define HS_RESPONSE_FLAG_DELETE 0x2

typedef struct _HS_FILTER_GLOBAL_DATA {
	PFLT_FILTER Filter;

	PFLT_PORT ServerPort;
	PFLT_PORT ClientPort;
	PEPROCESS ClientProcess;

	LONGLONG NextScanContextId;
	EX_PUSH_LOCK ScanContextListLock;
	LIST_ENTRY ScanContextList;

	BOOLEAN AllowUnload;
	BOOLEAN RegisteredObCallback;
	BOOLEAN RegisteredProcessCallback;
} HS_FILTER_GLOBAL_DATA, *PHS_FILTER_GLOBAL_DATA;

typedef struct _HS_SCAN_CONTEXT {
	PFLT_INSTANCE Instance;
	PFILE_OBJECT FileObject;
	PHS_SECTION_CONTEXT SectionContext;
	POBJECT_NAME_INFORMATION FileName;

	LONGLONG ScanId;
	LIST_ENTRY List;
} HS_SCAN_CONTEXT, *PHS_SCAN_CONTEXT;

typedef enum _HS_SCAN_REASON {
	HsScanOnPeOpen
} HS_SCAN_REASON;

typedef struct _HS_SCANNER_NOTIFICATION {
	HS_SCAN_REASON ScanReason;
	USHORT FileNameLength;
	LONGLONG ScanId;
} HS_SCANNER_NOTIFICATION, *PHS_SCANNER_NOTIFICATION;

typedef enum _HS_COMMAND {
	HsCmdCreateSectionForDataScan,
	HsCmdQueryFileName
} HS_COMMAND;

typedef struct _HS_SERVICE_COMMAND {
	HS_COMMAND Command;

	union
	{
		struct {
			LONGLONG ScanId;
		} CreateSectionForDataScan;

		struct {
			LONGLONG ScanId;
		} QueryFileName;

	};
} HS_SERVICE_COMMAND, *PHS_SERVICE_COMMAND;

extern HS_FILTER_GLOBAL_DATA GlobalData;

ULONG PsGetCurrentProcessSessionId(
	);

DRIVER_INITIALIZE DriverEntry;
NTSTATUS DriverEntry(
	_In_ PDRIVER_OBJECT DriverObject,
	_In_ PUNICODE_STRING RegistryPath
	);

NTSTATUS HsFilterUnload(
	_In_ FLT_FILTER_UNLOAD_FLAGS Flags
	);

NTSTATUS HsInstanceSetup(
	_In_ PCFLT_RELATED_OBJECTS FltObjects,
	_In_ FLT_INSTANCE_SETUP_FLAGS Flags,
	_In_ DEVICE_TYPE VolumeDeviceType,
	_In_ FLT_FILESYSTEM_TYPE VolumeFilesystemType
	);

NTSTATUS HsInstanceQueryTeardown(
	_In_ PCFLT_RELATED_OBJECTS FltObjects,
	_In_ FLT_INSTANCE_QUERY_TEARDOWN_FLAGS Flags
	);

FLT_POSTOP_CALLBACK_STATUS HsPostCreate(
	_Inout_ PFLT_CALLBACK_DATA Data,
	_In_ PCFLT_RELATED_OBJECTS FltObjects,
	_In_opt_ PVOID CompletionContext,
	_In_ FLT_POST_OPERATION_FLAGS Flags
	);

FLT_PREOP_CALLBACK_STATUS HsPreWrite(
	_Inout_ PFLT_CALLBACK_DATA Data,
	_In_ PCFLT_RELATED_OBJECTS FltObjects,
	_Flt_CompletionContext_Outptr_ PVOID* CompletionContext
	);

FLT_PREOP_CALLBACK_STATUS HsPreSetInformation(
	_Inout_ PFLT_CALLBACK_DATA Data,
	_In_ PCFLT_RELATED_OBJECTS FltObjects,
	_Flt_CompletionContext_Outptr_ PVOID* CompletionContext
	);

FLT_PREOP_CALLBACK_STATUS HsPreFsControl(
	_Inout_ PFLT_CALLBACK_DATA Data,
	_In_ PCFLT_RELATED_OBJECTS FltObjects,
	_Flt_CompletionContext_Outptr_ PVOID* CompletionContext
	);

FLT_PREOP_CALLBACK_STATUS HsPreAcquireForSectionSynchronization(
	_Inout_ PFLT_CALLBACK_DATA Data,
	_In_ PCFLT_RELATED_OBJECTS FltObjects,
	_Flt_CompletionContext_Outptr_ PVOID* CompletionContext
	);

FLT_PREOP_CALLBACK_STATUS HsPreCleanup(
	_Inout_ PFLT_CALLBACK_DATA Data,
	_In_ PCFLT_RELATED_OBJECTS FltObjects,
	_Flt_CompletionContext_Outptr_ PVOID* CompletionContext
	);

NTSTATUS HsFilterScanFile(
	_In_ PFLT_INSTANCE Instance,
	_In_ PFILE_OBJECT FileObject,
	_In_ HS_SCAN_REASON ScanReason,
	_Out_ PUCHAR ResponseFlags
	);

NTSTATUS HsFilterScanStream(
	_In_ PFLT_INSTANCE Instance,
	_In_ PFILE_OBJECT FileObject,
	_In_ PHS_STREAM_CONTEXT StreamContext,
	_In_ HS_SCAN_REASON ScanReason,
	_Out_ PBOOLEAN Infected
	);

NTSTATUS HsDeleteFile(
	_In_ PFLT_INSTANCE Instance,
	_In_ PFILE_OBJECT FileObject
	);

NTSTATUS HsSetStreamContextFlags(
	_In_ PFLT_INSTANCE Instance,
	_In_ PFILE_OBJECT FileObject,
	_In_ LONG FlagsToSet
	);

NTSTATUS HsGetFileCacheStatus(
	_In_ PFLT_INSTANCE Instance,
	_In_ PFILE_OBJECT FileObject,
	_Out_ PBOOLEAN Infected
	);

NTSTATUS HsSyncCache(
	_In_ PFLT_INSTANCE Instance,
	_In_ PFILE_OBJECT FileObject,
	_In_ PHS_STREAM_CONTEXT StreamContext
	);

VOID HsCreateProcessNotifyEx(
	_Inout_ PEPROCESS Process,
	_In_ HANDLE ProcessId,
	_Inout_opt_ PPS_CREATE_NOTIFY_INFO CreateInfo
	);

// Communication

NTSTATUS HsFilterCreateCommunicationPort(
	);

#endif