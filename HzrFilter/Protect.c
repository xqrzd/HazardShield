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

#include "Protect.h"
#include "Utility.h"

struct {
	RTL_AVL_TABLE ProtectedProcesses;
	PVOID RegistrationHandle;
	EX_PUSH_LOCK PushLock;
} CallbackInstance;

RTL_GENERIC_COMPARE_RESULTS AvlpCompareProcess(
	_In_ PRTL_AVL_TABLE Table,
	_In_ PPROTECTED_PROCESS Lhs,
	_In_ PPROTECTED_PROCESS Rhs)
{
	UNREFERENCED_PARAMETER(Table);

	if (Lhs->Process < Rhs->Process)
		return GenericLessThan;
	else if (Lhs->Process > Rhs->Process)
		return GenericGreaterThan;
	else
		return GenericEqual;
}

OB_PREOP_CALLBACK_STATUS HzrpObPreCallback(
	_In_ PVOID RegistrationContext,
	_Inout_ POB_PRE_OPERATION_INFORMATION OperationInformation)
{
	PEPROCESS process;
	ACCESS_MASK processAccessBitsToClear;
	ACCESS_MASK threadAccessBitsToClear;

	UNREFERENCED_PARAMETER(RegistrationContext);

	// ObRegisterCallbacks doesn't allow changing access of kernel handles
	if (OperationInformation->KernelHandle)
		return OB_PREOP_SUCCESS;

	// Get target process
	if (OperationInformation->ObjectType == *PsProcessType)
		process = OperationInformation->Object;
	else if (OperationInformation->ObjectType == *PsThreadType)
		process = IoThreadToProcess(OperationInformation->Object);
	else
		return OB_PREOP_SUCCESS; // Shouldn't ever happen

	// Allow process to open itself.
	if (process == IoGetCurrentProcess())
		return OB_PREOP_SUCCESS;

	if (HzrIsProcessProtected(process, &processAccessBitsToClear, &threadAccessBitsToClear))
	{
		ACCESS_MASK accessBitsToClear;

		if (OperationInformation->ObjectType == *PsProcessType)
			accessBitsToClear = processAccessBitsToClear;
		else
			accessBitsToClear = threadAccessBitsToClear;

		// Remove access flags
		if (OperationInformation->Operation == OB_OPERATION_HANDLE_CREATE)
		{
			OperationInformation->Parameters->CreateHandleInformation.DesiredAccess &= ~accessBitsToClear;
		}
		else if (OperationInformation->Operation == OB_OPERATION_HANDLE_DUPLICATE)
		{
			OperationInformation->Parameters->DuplicateHandleInformation.DesiredAccess &= ~accessBitsToClear;
		}
	}

	return OB_PREOP_SUCCESS;
}

VOID HzrpCreateProcessNotifyEx(
	_Inout_ PEPROCESS Process,
	_In_ HANDLE ProcessId,
	_Inout_opt_ PPS_CREATE_NOTIFY_INFO CreateInfo)
{
	UNREFERENCED_PARAMETER(ProcessId);

	if (!CreateInfo)
	{
		// Process is exiting, remove it from the protected processes.
		HzrRemoveProtectedProcess(Process);
	}
}

NTSTATUS HzrRegisterProtector()
{
	NTSTATUS status;
	OB_CALLBACK_REGISTRATION callbackRegistration;
	OB_OPERATION_REGISTRATION operationRegistration[2];

	operationRegistration[0].ObjectType = PsProcessType;
	operationRegistration[0].Operations = OB_OPERATION_HANDLE_CREATE | OB_OPERATION_HANDLE_DUPLICATE;
	operationRegistration[0].PreOperation = HzrpObPreCallback;
	operationRegistration[0].PostOperation = NULL;

	operationRegistration[1].ObjectType = PsThreadType;
	operationRegistration[1].Operations = OB_OPERATION_HANDLE_CREATE | OB_OPERATION_HANDLE_DUPLICATE;
	operationRegistration[1].PreOperation = HzrpObPreCallback;
	operationRegistration[1].PostOperation = NULL;

	callbackRegistration.Version = OB_FLT_REGISTRATION_VERSION;
	callbackRegistration.RegistrationContext = NULL;
	callbackRegistration.OperationRegistrationCount = ARRAYSIZE(operationRegistration);
	callbackRegistration.OperationRegistration = operationRegistration;

	RtlInitUnicodeString(&callbackRegistration.Altitude, L"40100.7");

	FltInitializePushLock(&CallbackInstance.PushLock);
	RtlInitializeGenericTableAvl(&CallbackInstance.ProtectedProcesses, AvlpCompareProcess, AvlAllocate, AvlFree, NULL);

	status = ObRegisterCallbacks(&callbackRegistration, &CallbackInstance.RegistrationHandle);

	if (NT_SUCCESS(status))
	{
		status = PsSetCreateProcessNotifyRoutineEx(HzrpCreateProcessNotifyEx, FALSE);
	}

	if (!NT_SUCCESS(status))
		FltDeletePushLock(&CallbackInstance.PushLock);

	return status;
}

VOID HzrUnRegisterProtector()
{
	ObUnRegisterCallbacks(CallbackInstance.RegistrationHandle);
	PsSetCreateProcessNotifyRoutineEx(HzrpCreateProcessNotifyEx, TRUE);

	FltAcquirePushLockExclusive(&CallbackInstance.PushLock);

	AvlDeleteAllElements(&CallbackInstance.ProtectedProcesses);

	FltReleasePushLock(&CallbackInstance.PushLock);
	FltDeletePushLock(&CallbackInstance.PushLock);
}

VOID HzrAddProtectedProcess(
	_In_ PEPROCESS Process,
	_In_ ACCESS_MASK ProcessAccessBitsToClear,
	_In_ ACCESS_MASK ThreadAccessBitsToClear)
{
	PROTECTED_PROCESS protectedProcess;

	protectedProcess.Process = Process;
	protectedProcess.ProcessAccessBitsToClear = ProcessAccessBitsToClear;
	protectedProcess.ThreadAccessBitsToClear = ThreadAccessBitsToClear;

	FltAcquirePushLockExclusive(&CallbackInstance.PushLock);

	RtlInsertElementGenericTableAvl(
		&CallbackInstance.ProtectedProcesses,
		&protectedProcess,
		sizeof(PROTECTED_PROCESS),
		NULL);

	FltReleasePushLock(&CallbackInstance.PushLock);
}

VOID HzrRemoveProtectedProcess(
	_In_ PEPROCESS Process)
{
	PROTECTED_PROCESS protectedProcess;

	protectedProcess.Process = Process;

	FltAcquirePushLockExclusive(&CallbackInstance.PushLock);

	RtlDeleteElementGenericTableAvl(&CallbackInstance.ProtectedProcesses, &protectedProcess);

	FltReleasePushLock(&CallbackInstance.PushLock);
}

BOOLEAN HzrIsProcessProtected(
	_In_ PEPROCESS Process,
	_Out_ PACCESS_MASK ProcessAccessBitsToClear,
	_Out_ PACCESS_MASK ThreadAccessBitsToClear)
{
	BOOLEAN found;
	PROTECTED_PROCESS searchKey;
	PPROTECTED_PROCESS protectedProcess;

	searchKey.Process = Process;

	FltAcquirePushLockShared(&CallbackInstance.PushLock);

	protectedProcess = RtlLookupElementGenericTableAvl(
		&CallbackInstance.ProtectedProcesses,
		&searchKey);

	found = protectedProcess != NULL;

	if (found)
	{
		*ProcessAccessBitsToClear = protectedProcess->ProcessAccessBitsToClear;
		*ThreadAccessBitsToClear = protectedProcess->ThreadAccessBitsToClear;
	}

	FltReleasePushLock(&CallbackInstance.PushLock);

	return found;
}