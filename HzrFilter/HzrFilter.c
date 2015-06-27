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

HS_FILTER_GLOBAL_DATA GlobalData;

CONST FLT_OPERATION_REGISTRATION Callbacks[] = {
	{
		IRP_MJ_CREATE,
		0,
		NULL,
		HsPostCreate
	},
	{
		IRP_MJ_WRITE,
		0,
		HsPreWrite,
		NULL
	},
	{
		IRP_MJ_SET_INFORMATION,
		0,
		HsPreSetInformation,
		NULL
	},
	{
		IRP_MJ_FILE_SYSTEM_CONTROL,
		0,
		HsPreFsControl,
		NULL
	},
	{
		IRP_MJ_CLEANUP,
		0,
		HsPreCleanup,
		NULL
	},
	{
		IRP_MJ_ACQUIRE_FOR_SECTION_SYNCHRONIZATION,
		0,
		HsPreAcquireForSectionSynchronization,
		NULL
	},

	{ IRP_MJ_OPERATION_END }
};

CONST FLT_REGISTRATION FilterRegistration = {
	sizeof(FLT_REGISTRATION),	// Size
	FLT_REGISTRATION_VERSION,	// Version
	0,	// Flags, FLTFL_REGISTRATION_DO_NOT_SUPPORT_SERVICE_STOP
	ContextRegistration,	// Context
	Callbacks,				// Operation callbacks
	HsFilterUnload,			// MiniFilterUnload
	HsInstanceSetup,		// InstanceSetup
	HsInstanceQueryTeardown,// InstanceQueryTeardown
	NULL,	// InstanceTeardownStart
	NULL,	// InstanceTeardownComplete
	NULL,	// GenerateFileName
	NULL,	// GenerateDestinationFileName
	NULL	// NormalizeNameComponent
};

NTSTATUS DriverEntry(
	_In_ PDRIVER_OBJECT DriverObject,
	_In_ PUNICODE_STRING RegistryPath)
{
	NTSTATUS status;

	UNREFERENCED_PARAMETER(RegistryPath);

	InitializeListHead(&GlobalData.ScanContextList);
	FltInitializePushLock(&GlobalData.ScanContextListLock);

	status = FltRegisterFilter(
		DriverObject,
		&FilterRegistration,
		&GlobalData.Filter);

	if (!NT_SUCCESS(status))
		return status;

	status = HsFilterCreateCommunicationPort();

	if (!NT_SUCCESS(status))
		goto End;

	status = FltStartFiltering(GlobalData.Filter);

	if (!NT_SUCCESS(status))
		goto End;

	status = HsRegisterProtector();

	if (NT_SUCCESS(status))
		GlobalData.RegisteredObCallback = TRUE;
	else
		goto End;

	status = PsSetCreateProcessNotifyRoutineEx(HsCreateProcessNotifyEx, FALSE);

	if (NT_SUCCESS(status))
		GlobalData.RegisteredProcessCallback = TRUE;
	else
		goto End;

End:
	if (!NT_SUCCESS(status))
	{
		// If any of the above calls failed, call the unload routine
		// to cleanup before exiting.

		HsFilterUnload(FLTFL_FILTER_UNLOAD_MANDATORY);
	}

	return status;
}

NTSTATUS HsFilterUnload(
	_In_ FLT_FILTER_UNLOAD_FLAGS Flags)
{
	if (FlagOn(Flags, FLTFL_FILTER_UNLOAD_MANDATORY) || GlobalData.AllowUnload)
	{
		if (GlobalData.ServerPort)
			FltCloseCommunicationPort(GlobalData.ServerPort);

		if (GlobalData.Filter)
			FltUnregisterFilter(GlobalData.Filter);

		if (GlobalData.RegisteredProcessCallback)
			PsSetCreateProcessNotifyRoutineEx(HsCreateProcessNotifyEx, TRUE);

		if (GlobalData.RegisteredObCallback)
			HsUnRegisterProtector();

		FltDeletePushLock(&GlobalData.ScanContextListLock);

		return STATUS_SUCCESS;
	}

	return STATUS_ACCESS_DENIED;
}

NTSTATUS HsInstanceSetup(
	_In_ PCFLT_RELATED_OBJECTS FltObjects,
	_In_ FLT_INSTANCE_SETUP_FLAGS Flags,
	_In_ DEVICE_TYPE VolumeDeviceType,
	_In_ FLT_FILESYSTEM_TYPE VolumeFilesystemType)
{
	NTSTATUS status;
	PHS_INSTANCE_CONTEXT instanceContext;

	UNREFERENCED_PARAMETER(Flags);

	status = FltAllocateContext(
		FltObjects->Filter,
		FLT_INSTANCE_CONTEXT,
		sizeof(HS_INSTANCE_CONTEXT),
		PagedPool,
		&instanceContext);

	if (NT_SUCCESS(status))
	{
		status = FltSetInstanceContext(
			FltObjects->Instance,
			FLT_SET_CONTEXT_KEEP_IF_EXISTS,
			instanceContext,
			NULL);

		if (NT_SUCCESS(status))
		{
			FltRegisterForDataScan(FltObjects->Instance);

			if (VolumeDeviceType == FILE_DEVICE_DISK_FILE_SYSTEM &&
				VolumeFilesystemType == FLT_FSTYPE_NTFS)
			{
				FltInitializePushLock(&instanceContext->CacheLock);

				RtlInitializeGenericTableAvl(
					&instanceContext->CacheTable,
					HsAvlCompareNtfsEntry,
					HsAvlAllocate,
					HsAvlFree,
					NULL);

				instanceContext->CacheSupported = TRUE;
			}
			else
				instanceContext->CacheSupported = FALSE;
		}

		// Always release the context, regardless of FltSetInstanceContext.
		// If FltSetInstanceContext succeeds, it takes a reference, and if
		// it fails, FltReleaseContext will delete the context.

		FltReleaseContext(instanceContext);
	}

	// Only attach to the volume if the instance context was set.
	return NT_SUCCESS(status) ? STATUS_SUCCESS : STATUS_FLT_DO_NOT_ATTACH;
}

NTSTATUS HsInstanceQueryTeardown(
	_In_ PCFLT_RELATED_OBJECTS FltObjects,
	_In_ FLT_INSTANCE_QUERY_TEARDOWN_FLAGS Flags)
{
	UNREFERENCED_PARAMETER(FltObjects);
	UNREFERENCED_PARAMETER(Flags);

	return STATUS_SUCCESS;
}

FLT_POSTOP_CALLBACK_STATUS HsPostCreate(
	_Inout_ PFLT_CALLBACK_DATA Data,
	_In_ PCFLT_RELATED_OBJECTS FltObjects,
	_In_opt_ PVOID CompletionContext,
	_In_ FLT_POST_OPERATION_FLAGS Flags)
{
	NTSTATUS status = Data->IoStatus.Status;
	ULONG_PTR stackLow;
	ULONG_PTR stackHigh;
	BOOLEAN isDirectory = FALSE;
	PFILE_OBJECT fileObject = Data->Iopb->TargetFileObject;

	UNREFERENCED_PARAMETER(CompletionContext);
	UNREFERENCED_PARAMETER(Flags);

	// Don't continue if the create has already failed.
	if (!NT_SUCCESS(status) || (status == STATUS_REPARSE))
		return FLT_POSTOP_FINISHED_PROCESSING;

	// Stack file objects are never scanned.
	IoGetStackLimits(&stackLow, &stackHigh);

	if (((ULONG_PTR)fileObject > stackLow) &&
		((ULONG_PTR)fileObject < stackHigh))
		return FLT_POSTOP_FINISHED_PROCESSING;

	// Directory opens don't need to be scanned.
	if (FlagOn(Data->Iopb->Parameters.Create.Options, FILE_DIRECTORY_FILE))
		return FLT_POSTOP_FINISHED_PROCESSING;

	// Skip pre-rename operations which always open a directory.
	if (FlagOn(Data->Iopb->OperationFlags, SL_OPEN_TARGET_DIRECTORY))
		return FLT_POSTOP_FINISHED_PROCESSING;

	// Skip paging files.
	if (FlagOn(Data->Iopb->OperationFlags, SL_OPEN_PAGING_FILE))
		return FLT_POSTOP_FINISHED_PROCESSING;

	// Skip scanning volume opens.
	if (FlagOn(FltObjects->FileObject->Flags, FO_VOLUME_OPEN))
		return FLT_POSTOP_FINISHED_PROCESSING;

	status = FltIsDirectory(FltObjects->FileObject, FltObjects->Instance, &isDirectory);

	// Directory opens don't need to be scanned.
	if (NT_SUCCESS(status) && isDirectory)
		return FLT_POSTOP_FINISHED_PROCESSING;

	// TODO: Check encrypted files here.

	if (HsIsPrefetchEcpPresent(FltObjects->Filter, Data))
	{
		// This file was opened by the prefetcher. Attach a stream
		// handle context and mark it as prefetcher opened, so that
		// future I/O on this file is ignored.

		PHS_STREAMHANDLE_CONTEXT streamHandleContext;

		status = FltAllocateContext(FltObjects->Filter,
			FLT_STREAMHANDLE_CONTEXT,
			sizeof(HS_STREAMHANDLE_CONTEXT),
			PagedPool,
			&streamHandleContext);

		if (NT_SUCCESS(status))
		{
			streamHandleContext->PrefetchOpen = TRUE;

			status = FltSetStreamHandleContext(
				FltObjects->Instance,
				FltObjects->FileObject,
				FLT_SET_CONTEXT_KEEP_IF_EXISTS,
				streamHandleContext,
				NULL);

			// Always release the context, regardless of FltSetInstanceContext.
			// If FltSetInstanceContext succeeds, it takes a reference, and if
			// it fails, FltReleaseContext will delete the context.

			FltReleaseContext(streamHandleContext);
		}
	}
	else
	{
		PHS_STREAM_CONTEXT streamContext;
		BOOLEAN infected = FALSE;

		// Check to see if there's already a stream context.

		status = FltGetStreamContext(
			FltObjects->Instance,
			FltObjects->FileObject,
			&streamContext);

		if (NT_SUCCESS(status))
		{
			infected = BooleanFlagOn(streamContext->Flags, HS_STREAM_FLAG_INFECTED);

			if (infected)
			{
				DbgPrint("Blocking create on infected stream context %wZ",
					&FltObjects->FileObject->FileName);
			}

			FltReleaseContext(streamContext);
		}
		else if (status == STATUS_NOT_FOUND)
		{
			status = FltAllocateContext(
				FltObjects->Filter,
				FLT_STREAM_CONTEXT,
				sizeof(HS_STREAM_CONTEXT),
				PagedPool,
				&streamContext);

			if (NT_SUCCESS(status))
			{
				// Initialize new stream context.

				streamContext->Flags = 0;
				FltInitializePushLock(&streamContext->ScanLock);

				// Don't store the return value, since even if
				// the cache fails, the scanning should continue.

				if (NT_SUCCESS(HsGetFileCacheStatus(
					FltObjects->Instance,
					FltObjects->FileObject,
					&infected)))
				{
					// Only scanned files are inserted into the cache.

					streamContext->Flags = HS_STREAM_FLAG_SCANNED;

					if (infected)
					{
						streamContext->Flags &= HS_STREAM_FLAG_INFECTED;

						DbgPrint("Blocking create on infected cached file %wZ",
							&FltObjects->FileObject->FileName);
					}
				}

				status = FltSetStreamContext(
					FltObjects->Instance,
					FltObjects->FileObject,
					FLT_SET_CONTEXT_KEEP_IF_EXISTS,
					streamContext,
					NULL);

				if (!NT_SUCCESS(status) && (status != STATUS_FLT_CONTEXT_ALREADY_DEFINED))
					DbgPrint("HsPostCreate::FltSetStreamContext failed %X", status);

				// Always release the context, regardless of FltSetInstanceContext.
				// If FltSetInstanceContext succeeds, it takes a reference, and if
				// it fails, FltReleaseContext will delete the context.

				FltReleaseContext(streamContext);
			}
		}

		if (infected)
		{
			FltCancelFileOpen(FltObjects->Instance, FltObjects->FileObject);

			Data->IoStatus.Status = STATUS_VIRUS_INFECTED;
			Data->IoStatus.Information = 0;
		}
	}

	return FLT_POSTOP_FINISHED_PROCESSING;
}

FLT_PREOP_CALLBACK_STATUS HsPreWrite(
	_Inout_ PFLT_CALLBACK_DATA Data,
	_In_ PCFLT_RELATED_OBJECTS FltObjects,
	_Flt_CompletionContext_Outptr_ PVOID* CompletionContext)
{
	UNREFERENCED_PARAMETER(Data);
	UNREFERENCED_PARAMETER(CompletionContext);

	HsSetStreamContextFlags(
		FltObjects->Instance,
		FltObjects->FileObject,
		HS_STREAM_FLAG_MODIFIED);

	return FLT_PREOP_SUCCESS_NO_CALLBACK;
}

FLT_PREOP_CALLBACK_STATUS HsPreSetInformation(
	_Inout_ PFLT_CALLBACK_DATA Data,
	_In_ PCFLT_RELATED_OBJECTS FltObjects,
	_Flt_CompletionContext_Outptr_ PVOID* CompletionContext)
{
	FILE_INFORMATION_CLASS fileInformationClass =
		Data->Iopb->Parameters.SetFileInformation.FileInformationClass;

	UNREFERENCED_PARAMETER(CompletionContext);

	if (fileInformationClass == FileEndOfFileInformation ||
		fileInformationClass == FileValidDataLengthInformation)
	{
		HsSetStreamContextFlags(
			FltObjects->Instance,
			FltObjects->FileObject,
			HS_STREAM_FLAG_MODIFIED);
	}

	return FLT_PREOP_SUCCESS_NO_CALLBACK;
}

FLT_PREOP_CALLBACK_STATUS HsPreFsControl(
	_Inout_ PFLT_CALLBACK_DATA Data,
	_In_ PCFLT_RELATED_OBJECTS FltObjects,
	_Flt_CompletionContext_Outptr_ PVOID* CompletionContext)
{
	UNREFERENCED_PARAMETER(CompletionContext);

	switch (Data->Iopb->Parameters.FileSystemControl.Common.FsControlCode)
	{
	case FSCTL_OFFLOAD_WRITE:
	case FSCTL_WRITE_RAW_ENCRYPTED:
	case FSCTL_SET_ZERO_DATA:
	{
		HsSetStreamContextFlags(
			FltObjects->Instance,
			FltObjects->FileObject,
			HS_STREAM_FLAG_MODIFIED);

		break;
	}
	}

	return FLT_PREOP_SUCCESS_NO_CALLBACK;
}

FLT_PREOP_CALLBACK_STATUS HsPreAcquireForSectionSynchronization(
	_Inout_ PFLT_CALLBACK_DATA Data,
	_In_ PCFLT_RELATED_OBJECTS FltObjects,
	_Flt_CompletionContext_Outptr_ PVOID* CompletionContext)
{
	UNREFERENCED_PARAMETER(CompletionContext);

	if (FlagOn(Data->Iopb->Parameters.AcquireForSectionSynchronization.PageProtection, PAGE_EXECUTE) &&
		Data->Iopb->Parameters.AcquireForSectionSynchronization.SyncType == SyncTypeCreateSection &&
		!HsIsPrefetchContextPresent(FltObjects->Instance, FltObjects->FileObject) &&
		IoGetCurrentProcess() != GlobalData.ClientProcess)
	{
		NTSTATUS status;
		PHS_STREAM_CONTEXT streamContext;
		BOOLEAN infected = FALSE;

		status = FltGetStreamContext(
			FltObjects->Instance,
			FltObjects->FileObject,
			&streamContext);

		if (NT_SUCCESS(status))
		{
			status = HsFilterScanStream(
				FltObjects->Instance,
				FltObjects->FileObject,
				streamContext,
				HsScanOnPeOpen,
				&infected);

			FltReleaseContext(streamContext);
		}
		else
		{
			DbgPrint("SectionSynchronization: Unable to get stream context %X for %wZ",
				status,
				&FltObjects->FileObject->FileName);
		}

		if (NT_SUCCESS(status) && infected)
		{
			Data->IoStatus.Status = STATUS_VIRUS_INFECTED;
			Data->IoStatus.Information = 0;

			// Stop processing for the operation and assign a final NTSTATUS value.

			return FLT_PREOP_COMPLETE;
		}
	}

	// Pass the I/O operation through without calling the
	// minifilter's postoperation callback on completion.

	return FLT_PREOP_SUCCESS_NO_CALLBACK;
}

FLT_PREOP_CALLBACK_STATUS HsPreCleanup(
	_Inout_ PFLT_CALLBACK_DATA Data,
	_In_ PCFLT_RELATED_OBJECTS FltObjects,
	_Flt_CompletionContext_Outptr_ PVOID* CompletionContext)
{
	NTSTATUS status;
	PHS_STREAMHANDLE_CONTEXT streamHandleContext;
	PHS_STREAM_CONTEXT streamContext;
	ULONG_PTR stackLow;
	ULONG_PTR stackHigh;

	UNREFERENCED_PARAMETER(Data);
	UNREFERENCED_PARAMETER(CompletionContext);

	// Skip scan on prefetcher handles to avoid deadlocks.

	status = FltGetStreamHandleContext(
		FltObjects->Instance,
		FltObjects->FileObject,
		&streamHandleContext);

	if (NT_SUCCESS(status))
	{
		if (streamHandleContext->PrefetchOpen)
		{
			// Because the Memory Manager can cache the file object
			// and use it for other applications performing mapped I/O,
			// whenever a cleanup operation is seen on a prefetcher
			// file object, that file object should no longer be
			// considered prefetcher-opened.

			FltDeleteContext(streamHandleContext);

			FltReleaseContext(streamHandleContext);

			return FLT_PREOP_SUCCESS_NO_CALLBACK;
		}

		FltReleaseContext(streamHandleContext);
	}

	// Stack file objects are never scanned.
	IoGetStackLimits(&stackLow, &stackHigh);

	if (((ULONG_PTR)FltObjects->FileObject > stackLow) &&
		((ULONG_PTR)FltObjects->FileObject < stackHigh))
		return FLT_PREOP_SUCCESS_NO_CALLBACK;

	// TODO: Check for encrypted files.

	status = FltGetStreamContext(
		FltObjects->Instance,
		FltObjects->FileObject,
		&streamContext);

	if (NT_SUCCESS(status))
	{
		if (FlagOn(streamContext->Flags, HS_STREAM_FLAG_MODIFIED))
		{
			// Scan write here.
		}

		// Only update the cache if the file has been scanned and
		// it hasn't been modified since.

		if (FlagOn(streamContext->Flags, HS_STREAM_FLAG_SCANNED) &&
			!FlagOn(streamContext->Flags, HS_STREAM_FLAG_MODIFIED))
		{
			HsSyncCache(
				FltObjects->Instance,
				FltObjects->FileObject,
				streamContext);
		}

		if (FlagOn(streamContext->Flags, HS_STREAM_FLAG_DELETE))
		{
			HsDeleteFile(FltObjects->Instance, FltObjects->FileObject);
		}

		FltReleaseContext(streamContext);
	}

	return FLT_PREOP_SUCCESS_NO_CALLBACK;
}

NTSTATUS HsFilterScanFile(
	_In_ PFLT_INSTANCE Instance,
	_In_ PFILE_OBJECT FileObject,
	_In_ HS_SCAN_REASON ScanReason,
	_Out_ PUCHAR ResponseFlags)
{
	NTSTATUS status;
	HS_SCAN_CONTEXT scanContext;
	HS_SCANNER_NOTIFICATION notification;
	ULONG replyLength;

	scanContext.Instance = Instance;
	scanContext.FileObject = FileObject;
	scanContext.SectionContext = NULL;

	FltAcquirePushLockExclusive(&GlobalData.ScanContextListLock);

	// Set a scan id and insert it into the global list.
	// The scan id is used by the user-mode service to
	// reference this scan context.

	scanContext.ScanId = (++GlobalData.NextScanContextId);
	InsertTailList(&GlobalData.ScanContextList, &scanContext.List);

	FltReleasePushLock(&GlobalData.ScanContextListLock);

	// Send notification to user-mode service.

	notification.ScanReason = ScanReason;
	notification.ScanId = scanContext.ScanId;
	replyLength = sizeof(UCHAR);

	status = FltSendMessage(
		GlobalData.Filter,
		&GlobalData.ClientPort,
		&notification,
		sizeof(notification),
		ResponseFlags,
		&replyLength,
		NULL);

	if (NT_SUCCESS(status))
	{
		// If the user-mode application created a section, clean
		// it up here. Note that the user-mode app is responsible
		// for closing SectionHandle.

		if (scanContext.SectionContext)
		{
			HsReleaseSectionContext(scanContext.SectionContext);
		}
	}
	else
		DbgPrint("FltSendMessage failed %X", status);

	// FltSendMessage waits for a response, so by this point the
	// user-application will be done with the scan context.

	FltAcquirePushLockExclusive(&GlobalData.ScanContextListLock);
	RemoveEntryList(&scanContext.List);
	FltReleasePushLock(&GlobalData.ScanContextListLock);

	return status;
}

NTSTATUS HsFilterScanStream(
	_In_ PFLT_INSTANCE Instance,
	_In_ PFILE_OBJECT FileObject,
	_In_ PHS_STREAM_CONTEXT StreamContext,
	_In_ HS_SCAN_REASON ScanReason,
	_Out_ PBOOLEAN Infected)
{
	NTSTATUS status;
	LONGLONG fileSize;

	status = HsGetFileSize(Instance, FileObject, &fileSize);

	if (NT_SUCCESS(status) &&
		fileSize > HS_MIN_FILE_SCAN_SIZE &&
		fileSize < HS_MAX_FILE_SCAN_SIZE)
	{
		// The file doesn't need to be scanned, as it is either
		// too small or too large.

		return STATUS_FILE_TOO_LARGE;
	}

	FltAcquirePushLockExclusive(&StreamContext->ScanLock);

	// Only scan the file if it hasn't already been scanned, or
	// if it has been modified. There is no need to check if it's
	// infected, as a file cannot be infected and not scanned.

	if (FlagOn(StreamContext->Flags, HS_STREAM_FLAG_MODIFIED) ||
		!FlagOn(StreamContext->Flags, HS_STREAM_FLAG_SCANNED))
	{
		UCHAR responseFlags;

		status = HsFilterScanFile(Instance, FileObject, ScanReason, &responseFlags);

		if (NT_SUCCESS(status))
		{
			RtlInterlockedSetBits(&StreamContext->Flags, HS_STREAM_FLAG_SCANNED);
			RtlInterlockedClearBits(&StreamContext->Flags, HS_STREAM_FLAG_MODIFIED);

			if (FlagOn(responseFlags, HS_RESPONSE_FLAG_INFECTED))
			{
				RtlInterlockedSetBits(&StreamContext->Flags, HS_STREAM_FLAG_INFECTED);
				*Infected = TRUE;
			}
			else
				*Infected = FALSE;

			if (FlagOn(responseFlags, HS_RESPONSE_FLAG_DELETE))
			{
				RtlInterlockedSetBits(&StreamContext->Flags, HS_STREAM_FLAG_DELETE);
			}
		}
	}
	else
	{
		// This stream doesn't need to be scanned.

		*Infected = BooleanFlagOn(StreamContext->Flags, HS_STREAM_FLAG_INFECTED);
		status = STATUS_SUCCESS;
	}

	FltReleasePushLock(&StreamContext->ScanLock);

	return status;
}

NTSTATUS HsDeleteFile(
	_In_ PFLT_INSTANCE Instance,
	_In_ PFILE_OBJECT FileObject)
{
	FILE_DISPOSITION_INFORMATION fileDispositionInfo;

	fileDispositionInfo.DeleteFile = TRUE;

	return FltSetInformationFile(
		Instance,
		FileObject,
		&fileDispositionInfo,
		sizeof(fileDispositionInfo),
		FileDispositionInformation);
}

NTSTATUS HsSetStreamContextFlags(
	_In_ PFLT_INSTANCE Instance,
	_In_ PFILE_OBJECT FileObject,
	_In_ LONG FlagsToSet)
{
	NTSTATUS status;
	PHS_STREAM_CONTEXT streamContext;

	status = FltGetStreamContext(Instance, FileObject, &streamContext);

	if (NT_SUCCESS(status))
	{
		RtlInterlockedSetBits(&streamContext->Flags, FlagsToSet);

		FltReleaseContext(streamContext);
	}

	return status;
}

NTSTATUS HsGetFileCacheStatus(
	_In_ PFLT_INSTANCE Instance,
	_In_ PFILE_OBJECT FileObject,
	_Out_ PBOOLEAN Infected)
{
	NTSTATUS status;
	PHS_INSTANCE_CONTEXT instanceContext;

	status = FltGetInstanceContext(Instance, &instanceContext);

	if (NT_SUCCESS(status))
	{
		if (instanceContext->CacheSupported)
		{
			FILE_INTERNAL_INFORMATION fileId;

			status = HsGetFileId64(Instance, FileObject, &fileId);

			if (NT_SUCCESS(status))
			{
				PHS_NTFS_CACHE_ENTRY entry;

				FltAcquirePushLockShared(&instanceContext->CacheLock);

				entry = RtlLookupElementGenericTableAvl(
					&instanceContext->CacheTable,
					&fileId.IndexNumber.QuadPart);

				if (entry)
					*Infected = entry->Infected;
				else
					status = STATUS_NOT_FOUND;

				FltReleasePushLock(&instanceContext->CacheLock);
			}
		}
		else
			status = STATUS_NOT_SUPPORTED;

		FltReleaseContext(instanceContext);
	}

	return status;
}

NTSTATUS HsSyncCache(
	_In_ PFLT_INSTANCE Instance,
	_In_ PFILE_OBJECT FileObject,
	_In_ PHS_STREAM_CONTEXT StreamContext)
{
	NTSTATUS status;
	PHS_INSTANCE_CONTEXT instanceContext;

	status = FltGetInstanceContext(Instance, &instanceContext);

	if (NT_SUCCESS(status))
	{
		if (instanceContext->CacheSupported)
		{
			HS_NTFS_CACHE_ENTRY entry;

			status = HsGetFileId64(Instance, FileObject, &entry.FileId);

			if (NT_SUCCESS(status))
			{
				PHS_NTFS_CACHE_ENTRY tableEntry;
				BOOLEAN inserted;

				FltAcquirePushLockExclusive(&instanceContext->CacheLock);

				tableEntry = RtlInsertElementGenericTableAvl(
					&instanceContext->CacheTable,
					&entry,
					sizeof(HS_NTFS_CACHE_ENTRY),
					&inserted);

				if (tableEntry)
				{
					tableEntry->Infected = BooleanFlagOn(
						StreamContext->Flags,
						HS_STREAM_FLAG_INFECTED);
				}

				FltReleasePushLock(&instanceContext->CacheLock);
			}
		}
		else
			status = STATUS_NOT_SUPPORTED;

		FltReleaseContext(instanceContext);
	}

	return status;
}

VOID HsCreateProcessNotifyEx(
	_Inout_ PEPROCESS Process,
	_In_ HANDLE ProcessId,
	_Inout_opt_ PPS_CREATE_NOTIFY_INFO CreateInfo)
{
	UNREFERENCED_PARAMETER(ProcessId);

	if (!CreateInfo)
	{
		// Process is exiting, remove it from the protected processes.
		HsUnProtectProcess(Process);
	}
}