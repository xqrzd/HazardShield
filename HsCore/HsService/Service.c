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

#include "Service.h"
#include "Scanner.h"
#include <Sfc.h>

struct {
	SERVICE_STATUS ServiceStatus;
	SERVICE_STATUS_HANDLE ServiceStatusHandle;
	SRWLOCK ScannerLock;
} ServiceData;

VOID WINAPI HsServiceMain(
	_In_ DWORD dwArgc,
	_In_ LPTSTR *lpszArgv)
{
	ServiceData.ServiceStatusHandle = RegisterServiceCtrlHandlerExW(
		HS_SERVICE_NAME,
		HsServiceHandlerEx,
		NULL);

	ServiceData.ServiceStatus.dwServiceType = SERVICE_WIN32_OWN_PROCESS;
	ServiceData.ServiceStatus.dwCurrentState = SERVICE_START_PENDING;
	ServiceData.ServiceStatus.dwControlsAccepted = SERVICE_ACCEPT_STOP;
	SetServiceStatus(ServiceData.ServiceStatusHandle, &ServiceData.ServiceStatus);

	// Init...

	ServiceData.ServiceStatus.dwCurrentState = SERVICE_RUNNING;
	SetServiceStatus(ServiceData.ServiceStatusHandle, &ServiceData.ServiceStatus);
}

DWORD WINAPI HsServiceHandlerEx(
	_In_ DWORD dwControl,
	_In_ DWORD dwEventType,
	_In_ LPVOID lpEventData,
	_In_ LPVOID lpContext)
{
	switch (dwControl)
	{
	case SERVICE_CONTROL_STOP:
	{
		ServiceData.ServiceStatus.dwCurrentState = SERVICE_STOP_PENDING;
		SetServiceStatus(ServiceData.ServiceStatusHandle, &ServiceData.ServiceStatus);

		// Cleanup...

		ServiceData.ServiceStatus.dwCurrentState = SERVICE_STOPPED;
		SetServiceStatus(ServiceData.ServiceStatusHandle, &ServiceData.ServiceStatus);

		return NO_ERROR;
	}

	case SERVICE_CONTROL_INTERROGATE:
		return NO_ERROR;
	}

	return ERROR_CALL_NOT_IMPLEMENTED;
}