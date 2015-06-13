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

typedef struct _HSP_PROTECTED_PROCESS {
	PEPROCESS Process;
	ACCESS_MASK ProcessAccessBitsToClear;
	ACCESS_MASK ThreadAccessBitsToClear;
} HSP_PROTECTED_PROCESS, *PHSP_PROTECTED_PROCESS;

RTL_GENERIC_COMPARE_RESULTS HspCompareProtectedProcess(
	_In_ PRTL_AVL_TABLE Table,
	_In_ PHSP_PROTECTED_PROCESS Lhs,
	_In_ PHSP_PROTECTED_PROCESS Rhs
	);

struct {
	RTL_AVL_TABLE ProtectedProcesses;
	EX_PUSH_LOCK ProtectedProcessLock;
	PVOID RegistrationHandle;
} ObCallbackInstance;

/// <summary>
/// This routine is called by the operating system
/// when a process or thread handle operation occurs.
/// </summary>
OB_PREOP_CALLBACK_STATUS HspObPreCallback(
	_In_ PVOID RegistrationContext,
	_Inout_ POB_PRE_OPERATION_INFORMATION OperationInformation)
{
	PEPROCESS process;
	ACCESS_MASK processAccessBitsToClear;
	ACCESS_MASK threadAccessBitsToClear;

	UNREFERENCED_PARAMETER(RegistrationContext);

	// ObRegisterCallbacks doesn't allow changing access of kernel handles.
	if (OperationInformation->KernelHandle)
		return OB_PREOP_SUCCESS;

	// Get target process.
	if (OperationInformation->ObjectType == *PsProcessType)
		process = OperationInformation->Object;
	else if (OperationInformation->ObjectType == *PsThreadType)
		process = IoThreadToProcess(OperationInformation->Object);
	else
		return OB_PREOP_SUCCESS; // Shouldn't ever happen.

	// Allow process to open itself.
	if (process == IoGetCurrentProcess())
		return OB_PREOP_SUCCESS;

	if (HsIsProcessProtected(process, &processAccessBitsToClear, &threadAccessBitsToClear))
	{
		ACCESS_MASK accessBitsToClear;

		if (OperationInformation->ObjectType == *PsProcessType)
			accessBitsToClear = processAccessBitsToClear;
		else
			accessBitsToClear = threadAccessBitsToClear;

		// Remove access flags.
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

/// <summary>
/// Starts filtering process and thread access rights.
/// </summary>
NTSTATUS HsRegisterProtector()
{
	NTSTATUS status;
	OB_CALLBACK_REGISTRATION callbackRegistration;
	OB_OPERATION_REGISTRATION operationRegistration[2];

	operationRegistration[0].ObjectType = PsProcessType;
	operationRegistration[0].Operations = OB_OPERATION_HANDLE_CREATE | OB_OPERATION_HANDLE_DUPLICATE;
	operationRegistration[0].PreOperation = HspObPreCallback;
	operationRegistration[0].PostOperation = NULL;

	operationRegistration[1].ObjectType = PsThreadType;
	operationRegistration[1].Operations = OB_OPERATION_HANDLE_CREATE | OB_OPERATION_HANDLE_DUPLICATE;
	operationRegistration[1].PreOperation = HspObPreCallback;
	operationRegistration[1].PostOperation = NULL;

	callbackRegistration.Version = OB_FLT_REGISTRATION_VERSION;
	callbackRegistration.RegistrationContext = NULL;
	callbackRegistration.OperationRegistrationCount = ARRAYSIZE(operationRegistration);
	callbackRegistration.OperationRegistration = operationRegistration;

	RtlInitUnicodeString(&callbackRegistration.Altitude, L"40100.7");

	FltInitializePushLock(&ObCallbackInstance.ProtectedProcessLock);

	RtlInitializeGenericTableAvl(
		&ObCallbackInstance.ProtectedProcesses,
		HspCompareProtectedProcess,
		AvlAllocate,
		AvlFree,
		NULL);

	status = ObRegisterCallbacks(&callbackRegistration, &ObCallbackInstance.RegistrationHandle);

	if (!NT_SUCCESS(status))
		FltDeletePushLock(&ObCallbackInstance.ProtectedProcessLock);

	return status;
}

/// <summary>
/// Stops process and thread access rights filtering.
/// </summary>
VOID HsUnRegisterProtector()
{
	ObUnRegisterCallbacks(ObCallbackInstance.RegistrationHandle);

	// If ObUnRegisterCallbacks waits for callbacks to finish processing
	// there is no need to lock here.

	FltAcquirePushLockExclusive(&ObCallbackInstance.ProtectedProcessLock);
	AvlDeleteAllElements(&ObCallbackInstance.ProtectedProcesses);
	FltReleasePushLock(&ObCallbackInstance.ProtectedProcessLock);
	FltDeletePushLock(&ObCallbackInstance.ProtectedProcessLock);
}

/// <summary>
/// Marks a process as protected. When this process or its threads are opened, the
/// given access rights will be stripped. Call HsUnProtectProcess when the process
/// no longer needs protection, or when it exits.
/// </summary>
/// <param name="Process">Pointer to the process object to protect.</param>
/// <param name="ProcessAccessBitsToClear">Process access rights to clear.</param>
/// <param name="ThreadAccessBitsToClear">Thread access rights to clear.</param>
VOID HsProtectProcess(
	_In_ PEPROCESS Process,
	_In_ ACCESS_MASK ProcessAccessBitsToClear,
	_In_ ACCESS_MASK ThreadAccessBitsToClear)
{
	HSP_PROTECTED_PROCESS protectedProcess;

	protectedProcess.Process = Process;
	protectedProcess.ProcessAccessBitsToClear = ProcessAccessBitsToClear;
	protectedProcess.ThreadAccessBitsToClear = ThreadAccessBitsToClear;

	FltAcquirePushLockExclusive(&ObCallbackInstance.ProtectedProcessLock);

	RtlInsertElementGenericTableAvl(
		&ObCallbackInstance.ProtectedProcesses,
		&protectedProcess,
		sizeof(HSP_PROTECTED_PROCESS),
		NULL);

	FltReleasePushLock(&ObCallbackInstance.ProtectedProcessLock);
}

/// <summary>
/// Removes a process from the list of protected processes.
/// </summary>
/// <param name="Process">Pointer to the process object to unprotect.</param>
VOID HsUnProtectProcess(
	_In_ PEPROCESS Process)
{
	HSP_PROTECTED_PROCESS protectedProcess;

	protectedProcess.Process = Process;

	FltAcquirePushLockExclusive(&ObCallbackInstance.ProtectedProcessLock);
	RtlDeleteElementGenericTableAvl(&ObCallbackInstance.ProtectedProcesses, &protectedProcess);
	FltReleasePushLock(&ObCallbackInstance.ProtectedProcessLock);
}

/// <summary>
/// Returns TRUE if the given process is protected, otherwise FALSE.
/// </summary>
/// <param name="Process">Pointer to a process object.</param>
/// <param name="ProcessAccessBitsToClear">Process access rights to clear.</param>
/// <param name="ThreadAccessBitsToClear">Thread access rights to clear.</param>
BOOLEAN HsIsProcessProtected(
	_In_ PEPROCESS Process,
	_Out_ PACCESS_MASK ProcessAccessBitsToClear,
	_Out_ PACCESS_MASK ThreadAccessBitsToClear)
{
	BOOLEAN found;
	HSP_PROTECTED_PROCESS searchKey;
	PHSP_PROTECTED_PROCESS protectedProcess;

	searchKey.Process = Process;

	FltAcquirePushLockShared(&ObCallbackInstance.ProtectedProcessLock);

	protectedProcess = RtlLookupElementGenericTableAvl(
		&ObCallbackInstance.ProtectedProcesses,
		&searchKey);

	found = protectedProcess != NULL;

	if (found)
	{
		*ProcessAccessBitsToClear = protectedProcess->ProcessAccessBitsToClear;
		*ThreadAccessBitsToClear = protectedProcess->ThreadAccessBitsToClear;
	}

	FltReleasePushLock(&ObCallbackInstance.ProtectedProcessLock);

	return found;
}

RTL_GENERIC_COMPARE_RESULTS HspCompareProtectedProcess(
	_In_ PRTL_AVL_TABLE Table,
	_In_ PHSP_PROTECTED_PROCESS Lhs,
	_In_ PHSP_PROTECTED_PROCESS Rhs)
{
	UNREFERENCED_PARAMETER(Table);

	if (Lhs->Process < Rhs->Process)
		return GenericLessThan;
	else if (Lhs->Process > Rhs->Process)
		return GenericGreaterThan;
	else
		return GenericEqual;
}