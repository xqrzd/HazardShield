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
	POB_CALLBACK_INSTANCE callbackInstance;
	PEPROCESS process;
	ACCESS_MASK processAccessBitsToClear;
	ACCESS_MASK threadAccessBitsToClear;

	callbackInstance = RegistrationContext;

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

	if (HzrIsProcessProtected(callbackInstance, process, &processAccessBitsToClear, &threadAccessBitsToClear))
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

NTSTATUS HzrRegisterProtector(
	_Out_ POB_CALLBACK_INSTANCE CallbackInstance)
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
	callbackRegistration.RegistrationContext = CallbackInstance;
	callbackRegistration.OperationRegistrationCount = ARRAYSIZE(operationRegistration);
	callbackRegistration.OperationRegistration = operationRegistration;

	RtlInitUnicodeString(&callbackRegistration.Altitude, L"40100.7");

	FltInitializePushLock(&CallbackInstance->ProtectedProcessLock);
	RtlInitializeGenericTableAvl(&CallbackInstance->ProtectedProcesses, AvlpCompareProcess, AvlAllocate, AvlFree, NULL);

	status = ObRegisterCallbacks(&callbackRegistration, &CallbackInstance->RegistrationHandle);

	if (!NT_SUCCESS(status))
		FltDeletePushLock(&CallbackInstance->ProtectedProcessLock);

	return status;
}

VOID HzrUnRegisterProtector(
	_In_ POB_CALLBACK_INSTANCE CallbackInstance)
{
	ObUnRegisterCallbacks(CallbackInstance->RegistrationHandle);

	FltAcquirePushLockExclusive(&CallbackInstance->ProtectedProcessLock);

	AvlDeleteAllElements(&CallbackInstance->ProtectedProcesses);

	FltReleasePushLock(&CallbackInstance->ProtectedProcessLock);
	FltDeletePushLock(&CallbackInstance->ProtectedProcessLock);
}

VOID HzrAddProtectedProcess(
	_In_ POB_CALLBACK_INSTANCE CallbackInstance,
	_In_ PEPROCESS Process,
	_In_ ACCESS_MASK ProcessAccessBitsToClear,
	_In_ ACCESS_MASK ThreadAccessBitsToClear)
{
	PROTECTED_PROCESS protectedProcess;

	protectedProcess.Process = Process;
	protectedProcess.ProcessAccessBitsToClear = ProcessAccessBitsToClear;
	protectedProcess.ThreadAccessBitsToClear = ThreadAccessBitsToClear;

	FltAcquirePushLockExclusive(&CallbackInstance->ProtectedProcessLock);

	RtlInsertElementGenericTableAvl(
		&CallbackInstance->ProtectedProcesses,
		&protectedProcess,
		sizeof(PROTECTED_PROCESS),
		NULL);

	FltReleasePushLock(&CallbackInstance->ProtectedProcessLock);
}

VOID HzrRemoveProtectedProcess(
	_In_ POB_CALLBACK_INSTANCE CallbackInstance,
	_In_ PEPROCESS Process)
{
	PROTECTED_PROCESS protectedProcess;

	protectedProcess.Process = Process;

	FltAcquirePushLockExclusive(&CallbackInstance->ProtectedProcessLock);

	RtlDeleteElementGenericTableAvl(&CallbackInstance->ProtectedProcesses, &protectedProcess);

	FltReleasePushLock(&CallbackInstance->ProtectedProcessLock);
}

BOOLEAN HzrIsProcessProtected(
	_In_ POB_CALLBACK_INSTANCE CallbackInstance,
	_In_ PEPROCESS Process,
	_Out_ PACCESS_MASK ProcessAccessBitsToClear,
	_Out_ PACCESS_MASK ThreadAccessBitsToClear)
{
	BOOLEAN found;
	PROTECTED_PROCESS searchKey;
	PPROTECTED_PROCESS protectedProcess;

	searchKey.Process = Process;

	FltAcquirePushLockShared(&CallbackInstance->ProtectedProcessLock);

	protectedProcess = RtlLookupElementGenericTableAvl(
		&CallbackInstance->ProtectedProcesses,
		&searchKey);

	found = protectedProcess != NULL;

	if (found)
	{
		*ProcessAccessBitsToClear = protectedProcess->ProcessAccessBitsToClear;
		*ThreadAccessBitsToClear = protectedProcess->ThreadAccessBitsToClear;
	}

	FltReleasePushLock(&CallbackInstance->ProtectedProcessLock);

	return found;
}