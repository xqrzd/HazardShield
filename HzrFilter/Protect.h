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

#ifndef HZRFILTER_PROTECT_H
#define HZRFILTER_PROTECT_H

#include <fltKernel.h>

typedef struct _OB_CALLBACK_INSTANCE {
	RTL_AVL_TABLE ProtectedProcesses;
	EX_PUSH_LOCK ProtectedProcessLock;
	PVOID RegistrationHandle;
} OB_CALLBACK_INSTANCE, *POB_CALLBACK_INSTANCE;

typedef struct _PROTECTED_PROCESS {
	PEPROCESS Process;
	ACCESS_MASK ProcessAccessBitsToClear;
	ACCESS_MASK ThreadAccessBitsToClear;
} PROTECTED_PROCESS, *PPROTECTED_PROCESS;

NTSTATUS HzrRegisterProtector(
	_Out_ POB_CALLBACK_INSTANCE CallbackInstance
	);

VOID HzrUnRegisterProtector(
	_In_ POB_CALLBACK_INSTANCE CallbackInstance
	);

VOID HzrAddProtectedProcess(
	_In_ POB_CALLBACK_INSTANCE CallbackInstance,
	_In_ PEPROCESS Process,
	_In_ ACCESS_MASK ProcessAccessBitsToClear,
	_In_ ACCESS_MASK ThreadAccessBitsToClear
	);

VOID HzrRemoveProtectedProcess(
	_In_ POB_CALLBACK_INSTANCE CallbackInstance,
	_In_ PEPROCESS Process
	);

BOOLEAN HzrIsProcessProtected(
	_In_ POB_CALLBACK_INSTANCE CallbackInstance,
	_In_ PEPROCESS Process,
	_Out_ PACCESS_MASK ProcessAccessBitsToClear,
	_Out_ PACCESS_MASK ThreadAccessBitsToClear
	);

#endif