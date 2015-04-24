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

#include "Behavior.h"
#include "Utility.h"

BOOLEAN HspIsChildOfMonitoredProcess(
	_In_ HANDLE CreatorProcessId,
	_Out_ PHS_PROCESS_TYPE Type
	);

HS_PROCESS_TYPE HspShouldMonitorProcess(
	_In_ PCUNICODE_STRING ImageFileName,
	_In_opt_ PCUNICODE_STRING CommandLine
	);

RTL_GENERIC_COMPARE_RESULTS HspCompareMonitoredProcess(
	_In_ PRTL_AVL_TABLE Table,
	_In_ PHS_MONITORED_PROCESS Lhs,
	_In_ PHS_MONITORED_PROCESS Rhs
	);

CONST UNICODE_STRING FIREFOX_PLUGIN_HOST = RTL_CONSTANT_STRING(L"plugin-container.exe");

struct {
	RTL_AVL_TABLE Processes;
	EX_PUSH_LOCK ProcessLock;
} BehaviorInstance;

VOID HsInitializeBehaviorSystem()
{
	FltInitializePushLock(&BehaviorInstance.ProcessLock);
	RtlInitializeGenericTableAvl(&BehaviorInstance.Processes, HspCompareMonitoredProcess, AvlAllocate, AvlFree, NULL);
}

VOID HsDeleteBehaviorSystem()
{
	FltAcquirePushLockExclusive(&BehaviorInstance.ProcessLock);

	AvlDeleteAllElements(&BehaviorInstance.Processes);

	FltReleasePushLock(&BehaviorInstance.ProcessLock);
	FltDeletePushLock(&BehaviorInstance.ProcessLock);
}

BOOLEAN HsMonitorPreWrite(
	_Inout_ PFLT_CALLBACK_DATA Data,
	_In_ PCFLT_RELATED_OBJECTS FltObjects)
{
	BOOLEAN block;
	HS_PROCESS_TYPE type;

	UNREFERENCED_PARAMETER(FltObjects);

	block = FALSE;
	type = HsIsProcessMonitored(IoGetCurrentProcess());

	switch (type)
	{
	case HS_PROCESS_TYPE_FIREFOX_PLUGIN_HOST:
	{
		if (Data->Iopb->Parameters.Write.ByteOffset.QuadPart == 0 &&
			Data->Iopb->Parameters.Write.Length >= 2)
		{
			PVOID buffer = HzrFilterGetBuffer(
				Data->Iopb->Parameters.Write.WriteBuffer,
				Data->Iopb->Parameters.Write.MdlAddress);

			if (buffer)
			{
				if (RtlEqualMemory(buffer, "MZ", 2))
				{
					block = TRUE;
				}
			}
		}

		break;
	}
	}

	return block;
}

VOID HsMonitorCreateProcessNotifyEx(
	_Inout_ PEPROCESS Process,
	_Inout_opt_ PPS_CREATE_NOTIFY_INFO CreateInfo)
{
	if (CreateInfo)
	{
		HS_PROCESS_TYPE type;

		// If a monitored process is creating a child, monitor the child.
		if (HspIsChildOfMonitoredProcess(CreateInfo->CreatingThreadId.UniqueProcess, &type))
		{
			DbgPrint("Monitoring child process %wZ", CreateInfo->ImageFileName);

			HsMonitorProcess(Process, type);

			return;
		}

		if (CreateInfo->FileOpenNameAvailable)
		{
			type = HspShouldMonitorProcess(CreateInfo->ImageFileName, CreateInfo->CommandLine);

			if (type)
			{
				DbgPrint("Monitoring process %wZ", CreateInfo->ImageFileName);

				HsMonitorProcess(Process, type);
			}
		}
		else
			DbgPrint("HsMonitorCreateProcessNotifyEx: File name not available for EPROCESS %016llX", Process);
	}
	else
	{
		HsUnMonitorProcess(Process);
	}
}

VOID HsMonitorProcess(
	_In_ PEPROCESS Process,
	_In_ HS_PROCESS_TYPE Type)
{
	HS_MONITORED_PROCESS process;

	process.Process = Process;
	process.Type = Type;

	FltAcquirePushLockExclusive(&BehaviorInstance.ProcessLock);

	RtlInsertElementGenericTableAvl(
		&BehaviorInstance.Processes,
		&process,
		sizeof(HS_MONITORED_PROCESS),
		NULL);

	FltReleasePushLock(&BehaviorInstance.ProcessLock);
}

VOID HsUnMonitorProcess(
	_In_ PEPROCESS Process)
{
	HS_MONITORED_PROCESS process;

	process.Process = Process;

	FltAcquirePushLockExclusive(&BehaviorInstance.ProcessLock);

	RtlDeleteElementGenericTableAvl(&BehaviorInstance.Processes, &process);

	FltReleasePushLock(&BehaviorInstance.ProcessLock);
}

HS_PROCESS_TYPE HsIsProcessMonitored(
	_In_ PEPROCESS Process)
{
	HS_MONITORED_PROCESS searchKey;
	PHS_MONITORED_PROCESS process;
	HS_PROCESS_TYPE processType;

	searchKey.Process = Process;

	FltAcquirePushLockShared(&BehaviorInstance.ProcessLock);

	process = RtlLookupElementGenericTableAvl(
		&BehaviorInstance.Processes,
		&searchKey);

	if (process)
		processType = process->Type;
	else
		processType = HS_PROCESS_TYPE_NONE;

	FltReleasePushLock(&BehaviorInstance.ProcessLock);

	return processType;
}

BOOLEAN HspIsChildOfMonitoredProcess(
	_In_ HANDLE CreatorProcessId,
	_Out_ PHS_PROCESS_TYPE Type)
{
	NTSTATUS status;
	PEPROCESS creatorProcess;
	BOOLEAN isChild = FALSE;

	status = PsLookupProcessByProcessId(CreatorProcessId, &creatorProcess);

	if (NT_SUCCESS(status))
	{
		*Type = HsIsProcessMonitored(creatorProcess);

		if (*Type)
			isChild = TRUE;

		ObDereferenceObject(creatorProcess);
	}

	return isChild;
}

HS_PROCESS_TYPE HspShouldMonitorProcess(
	_In_ PCUNICODE_STRING ImageFileName,
	_In_opt_ PCUNICODE_STRING CommandLine)
{
	UNICODE_STRING fileName;

	UNREFERENCED_PARAMETER(CommandLine);

	if (HzrGetFileNameFromPath(ImageFileName, &fileName))
	{
		if (RtlEqualUnicodeString(&fileName, &FIREFOX_PLUGIN_HOST, FALSE))
		{
			return HS_PROCESS_TYPE_FIREFOX_PLUGIN_HOST;
		}
	}
	else
		DbgPrint("Unable to get file name from %wZ", ImageFileName);

	return HS_PROCESS_TYPE_NONE;
}

RTL_GENERIC_COMPARE_RESULTS HspCompareMonitoredProcess(
	_In_ PRTL_AVL_TABLE Table,
	_In_ PHS_MONITORED_PROCESS Lhs,
	_In_ PHS_MONITORED_PROCESS Rhs)
{
	UNREFERENCED_PARAMETER(Table);

	if (Lhs->Process < Rhs->Process)
		return GenericLessThan;
	else if (Lhs->Process > Rhs->Process)
		return GenericGreaterThan;
	else
		return GenericEqual;
}