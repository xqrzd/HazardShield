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

#pragma once

#include "Utility.h"

#define SE_LOAD_DRIVER_PRIVILEGE 10

DWORD HzrGetProcessorCount()
{
	SYSTEM_INFO systemInfo;

	GetSystemInfo(&systemInfo);

	return systemInfo.dwNumberOfProcessors;
}

VOID HzrEnableLoadDriverPrivilege()
{
	HANDLE tokenHandle;

	if (OpenProcessToken(
		GetCurrentProcess(),
		TOKEN_ADJUST_PRIVILEGES,
		&tokenHandle))
	{
		TOKEN_PRIVILEGES privileges;

		privileges.PrivilegeCount = 1;

		privileges.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;
		privileges.Privileges[0].Luid.LowPart = SE_LOAD_DRIVER_PRIVILEGE;
		privileges.Privileges[0].Luid.HighPart = 0;

		AdjustTokenPrivileges(
			tokenHandle,
			FALSE,
			&privileges,
			0,
			NULL,
			NULL);

		CloseHandle(tokenHandle);
	}
}