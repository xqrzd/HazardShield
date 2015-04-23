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

CONST UNICODE_STRING FirefoxPluginHost = RTL_CONSTANT_STRING(L"plugin-container.exe");
CONST UNICODE_STRING Opera = RTL_CONSTANT_STRING(L"chrome.exe");
CONST UNICODE_STRING Chrome = RTL_CONSTANT_STRING(L"opera.exe");

struct {
	RTL_AVL_TABLE Processes;
	EX_PUSH_LOCK ProcessLock;
} BehaviorInstance;

RTL_GENERIC_COMPARE_RESULTS AvlpCompareExplProcess(
	_In_ PRTL_AVL_TABLE Table,
	_In_ PEXPL_PROCESS Lhs,
	_In_ PEXPL_PROCESS Rhs)
{
	UNREFERENCED_PARAMETER(Table);

	if (Lhs->Process < Rhs->Process)
		return GenericLessThan;
	else if (Lhs->Process > Rhs->Process)
		return GenericGreaterThan;
	else
		return GenericEqual;
}

VOID HzrExplInit()
{
	FltInitializePushLock(&BehaviorInstance.ProcessLock);
	RtlInitializeGenericTableAvl(&BehaviorInstance.Processes, AvlpCompareExplProcess, AvlAllocate, AvlFree, NULL);
}

VOID HzrExplFree()
{
	FltAcquirePushLockExclusive(&BehaviorInstance.ProcessLock);

	AvlDeleteAllElements(&BehaviorInstance.Processes);

	FltReleasePushLock(&BehaviorInstance.ProcessLock);
	FltDeletePushLock(&BehaviorInstance.ProcessLock);
}

BOOLEAN HzrExplPreWrite(
	_Inout_ PFLT_CALLBACK_DATA Data,
	_In_ PCFLT_RELATED_OBJECTS FltObjects)
{
	PEPROCESS process = IoGetCurrentProcess();
	BOOLEAN block = FALSE;
	EXPL_PROCESS_TYPE type;

	UNREFERENCED_PARAMETER(FltObjects);

	type = HzrExplIsProcessExploit(process);

	switch (type)
	{
	case EXPL_PROCESS_JAVA:
	case EXPL_PROCESS_FIREFOX_PLUGIN_HOST:
	case EXPL_PROCESS_CHROME_OPERA_PLUGIN_HOST:
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

BOOLEAN HzrExplpIsChildOfExploitProcess(
	_In_ HANDLE CreatorProcessId,
	_Out_ PEXPL_PROCESS_TYPE Type)
{
	NTSTATUS status;
	PEPROCESS creatorProcess;
	BOOLEAN isChild = FALSE;

	status = PsLookupProcessByProcessId(CreatorProcessId, &creatorProcess);

	if (NT_SUCCESS(status))
	{
		*Type = HzrExplIsProcessExploit(creatorProcess);

		if (*Type)
			isChild = TRUE;

		ObDereferenceObject(creatorProcess);
	}

	return isChild;
}

VOID HzrExplCreateProcessNotifyEx(
	_Inout_ PEPROCESS Process,
	_Inout_opt_ PPS_CREATE_NOTIFY_INFO CreateInfo)
{
	if (CreateInfo)
	{
		EXPL_PROCESS_TYPE type;

		// If a monitored process is creating a child, monitor the child.
		if (HzrExplpIsChildOfExploitProcess(CreateInfo->CreatingThreadId.UniqueProcess, &type))
		{
			DbgPrint("Adding child exploit process %wZ", CreateInfo->ImageFileName);

			HzrExplAddExploitProcess(Process, type);

			return;
		}

		if (CreateInfo->FileOpenNameAvailable)
		{
			type = HzrExplCheckNewProcess(CreateInfo->ImageFileName, CreateInfo->CommandLine);

			if (type)
			{
				DbgPrint("Adding exploit process %wZ", CreateInfo->ImageFileName);

				HzrExplAddExploitProcess(Process, type);
			}
		}
		else
			DbgPrint("HzrExplCreateProcessNotifyEx: File name not available for EPROCESS %016llX", Process);
	}
	else
	{
		HzrExplRemoveExploitProcess(Process);
	}
}

EXPL_PROCESS_TYPE HzrExplCheckNewProcess(
	_In_ PCUNICODE_STRING ImageFileName,
	_In_opt_ PCUNICODE_STRING CommandLine)
{
	UNICODE_STRING fileName;

	UNREFERENCED_PARAMETER(CommandLine);

	if (HzrGetFileNameFromPath(ImageFileName, &fileName))
	{
		if (RtlEqualUnicodeString(&fileName, &FirefoxPluginHost, FALSE))
		{
			return EXPL_PROCESS_FIREFOX_PLUGIN_HOST;
		}

		/*if (RtlEqualUnicodeString(&fileName, &Chrome, FALSE) ||
			RtlEqualUnicodeString(&fileName, &Opera, FALSE))
		{
			return EXPL_PROCESS_CHROME_OPERA_PLUGIN_HOST;
		}*/
	}
	else
		DbgPrint("Unable to get file name from %wZ", ImageFileName);

	return EXPL_PROCESS_NONE;
}

VOID HzrExplAddExploitProcess(
	_In_ PEPROCESS Process,
	_In_ EXPL_PROCESS_TYPE Type)
{
	EXPL_PROCESS exploitProcess;

	exploitProcess.Process = Process;
	exploitProcess.Type = Type;

	FltAcquirePushLockExclusive(&BehaviorInstance.ProcessLock);

	RtlInsertElementGenericTableAvl(
		&BehaviorInstance.Processes,
		&exploitProcess,
		sizeof(EXPL_PROCESS),
		NULL);

	FltReleasePushLock(&BehaviorInstance.ProcessLock);
}

VOID HzrExplRemoveExploitProcess(
	_In_ PEPROCESS Process)
{
	EXPL_PROCESS exploitProcess;

	exploitProcess.Process = Process;

	FltAcquirePushLockExclusive(&BehaviorInstance.ProcessLock);

	RtlDeleteElementGenericTableAvl(&BehaviorInstance.Processes, &exploitProcess);

	FltReleasePushLock(&BehaviorInstance.ProcessLock);
}

EXPL_PROCESS_TYPE HzrExplIsProcessExploit(
	_In_ PEPROCESS Process)
{
	EXPL_PROCESS searchKey;
	PEXPL_PROCESS exploitProcess;
	EXPL_PROCESS_TYPE processType;

	searchKey.Process = Process;

	FltAcquirePushLockShared(&BehaviorInstance.ProcessLock);

	exploitProcess = RtlLookupElementGenericTableAvl(
		&BehaviorInstance.Processes,
		&searchKey);

	if (exploitProcess)
		processType = exploitProcess->Type;
	else
		processType = EXPL_PROCESS_NONE;

	FltReleasePushLock(&BehaviorInstance.ProcessLock);

	return processType;
}