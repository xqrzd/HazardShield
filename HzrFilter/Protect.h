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

/// <summary>
/// Starts filtering process and thread access rights.
/// </summary>
NTSTATUS HsRegisterProtector(
	);

/// <summary>
/// Stops process and thread access rights filtering.
/// </summary>
VOID HsUnRegisterProtector(
	);

/// <summary>
/// Marks a process as protected. When this process or its threads are opened, the
/// given access rights will be cleared. Call HsUnProtectProcess when the process
/// no longer needs protection, or when it exits.
/// </summary>
/// <param name="Process">Pointer to the process object to protect.</param>
/// <param name="ProcessAccessBitsToClear">Process access rights to clear.</param>
/// <param name="ThreadAccessBitsToClear">Thread access rights to clear.</param>
VOID HsProtectProcess(
	_In_ PEPROCESS Process,
	_In_ ACCESS_MASK ProcessAccessBitsToClear,
	_In_ ACCESS_MASK ThreadAccessBitsToClear
	);

/// <summary>
/// Removes a process from the list of protected processes.
/// </summary>
/// <param name="Process">Pointer to the process object to unprotect.</param>
VOID HsUnProtectProcess(
	_In_ PEPROCESS Process
	);

/// <summary>
/// Returns TRUE if the given process is protected, otherwise FALSE.
/// </summary>
/// <param name="Process">Pointer to a process object.</param>
/// <param name="ProcessAccessBitsToClear">Process access rights to clear.</param>
/// <param name="ThreadAccessBitsToClear">Thread access rights to clear.</param>
BOOLEAN HsIsProcessProtected(
	_In_ PEPROCESS Process,
	_Out_ PACCESS_MASK ProcessAccessBitsToClear,
	_Out_ PACCESS_MASK ThreadAccessBitsToClear
	);

#endif