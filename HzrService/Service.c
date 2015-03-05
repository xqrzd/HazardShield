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

#include "clamav.h"
#include "DriverHandler.h"
#include "Logger.h"
#include "Service.h"
#include "Utility.h"
#include <Sfc.h>

struct {
	DRIVER_INSTANCE DriverInstance;
	SERVICE_STATUS ServiceStatus;
	SERVICE_STATUS_HANDLE ServiceStatusHandle;
	struct cl_engine* Engine;
	SRWLOCK EngineLock;
	INT SignatureCount;
} ServiceData;

BOOLEAN HzrpScanBuffer(
	_In_ PVOID Buffer,
	_In_ ULONG BufferSize,
	_Out_ PCHAR* ThreatName)
{
	BOOLEAN infected = FALSE;

	cl_fmap_t* map = cl_fmap_open_memory(Buffer, BufferSize);
	if (map)
	{
		INT result;

		AcquireSRWLockShared(&ServiceData.EngineLock);

		result = cl_scanmap_callback(
			map,
			ThreatName,
			NULL,
			ServiceData.Engine,
			CL_SCAN_STDOPT,
			NULL);

		ReleaseSRWLockShared(&ServiceData.EngineLock);

		cl_fmap_close(map);

		if (result == CL_VIRUS)
			infected = TRUE;
		else if (result != CL_CLEAN)
			LogMessageA("HzrpScanBuffer::cl_scanmap_callback failed %d", result);
	}
	else
		LogMessageA("HzrpScanBuffer::cl_fmap_open_memory failed");

	return infected;
}

BOOLEAN FileCallback(
	_In_ PFILE_INFO FileInfo)
{
	BOOLEAN flags;
	BOOLEAN infected;
	PCHAR threatName;

	// Ignore system files.
	if (SfcIsFileProtected(NULL, FileInfo->FileName))
		return RESPONSE_FLAG_WHITELIST;

	infected = HzrpScanBuffer(FileInfo->FileData, FileInfo->FileSize, &threatName);

	if (infected)
	{
		LogMessageA("Found %s in %S", threatName, FileInfo->FileName);
		flags = RESPONSE_FLAG_BLOCK_OPERATION;
	}
	else
		flags = RESPONSE_FLAG_WHITELIST;

	return flags;
}

VOID HzrpInitClamAV()
{
	INT result;
	CHAR path[MAX_PATH];

	ServiceData.Engine = cl_engine_new();

	if (ServiceData.Engine)
	{
		if (ExpandEnvironmentStringsA(CLAMAV_DATABASE_PATH, path, ARRAYSIZE(path)))
		{
			ServiceData.SignatureCount = 0;
			result = cl_load(path, ServiceData.Engine, &ServiceData.SignatureCount, CL_DB_BYTECODE);

			if (result == CL_SUCCESS)
			{
				//LogMessageA("HzrInitClamAV: Loaded %u signatures", ServiceData.SignatureCount);
				result = cl_engine_compile(ServiceData.Engine);
				if (result != CL_SUCCESS)
					LogMessageA("HzrpInitClamAV::cl_engine_compile failed %d", result);
			}
			else
				LogMessageA("HzrpInitClamAV::cl_load failed %d", result);
		}
		else
			LogMessageA("HzrpInitClamAV::ExpandEnvironmentStrings failed on CLAMAV_DATABASE_PATH %u", GetLastError());
	}
	else
		LogMessageA("HzrpInitClamAV::cl_engine_new failed");
}

FORCEINLINE VOID HzrInit()
{
	HRESULT result;

	HzrEnableLoadDriverPrivilege();

	result = DrvLoad(FILTER_NAME);

	if (!SUCCEEDED(result))
	{
		// Check if minifilter is already running.
		if (result != HRESULT_FROM_WIN32(ERROR_ALREADY_EXISTS) &&
			result != HRESULT_FROM_WIN32(ERROR_SERVICE_ALREADY_RUNNING))
		{
			LogMessageA("HzrInit::DrvLoad failed %X", result);
			return;
		}
	}

	if (DrvConnect(PORT_NAME, HzrGetProcessorCount(), FileCallback, &ServiceData.DriverInstance))
	{
		InitializeSRWLock(&ServiceData.EngineLock);
		cl_init(CL_INIT_DEFAULT);
		HzrpInitClamAV();

		DrvStartEventMonitor(&ServiceData.DriverInstance);
	}
}

VOID HzrServiceCleanup()
{
	DrvUnload(&ServiceData.DriverInstance, FILTER_NAME);

	DrvDisconnect(&ServiceData.DriverInstance);

	if (ServiceData.Engine)
		cl_engine_free(ServiceData.Engine);
}

//void main()
//{
//	HzrInit();
//	getchar();
//}

VOID WINAPI HzrServiceMain(
	_In_ DWORD dwArgc,
	_In_ LPTSTR *lpszArgv)
{
	ServiceData.ServiceStatusHandle = RegisterServiceCtrlHandlerEx(SERVICE_NAME, HzrHandlerEx, NULL);

	ServiceData.ServiceStatus.dwServiceType = SERVICE_WIN32_OWN_PROCESS;
	ServiceData.ServiceStatus.dwCurrentState = SERVICE_START_PENDING;
	ServiceData.ServiceStatus.dwControlsAccepted = SERVICE_ACCEPT_STOP;
	SetServiceStatus(ServiceData.ServiceStatusHandle, &ServiceData.ServiceStatus);

	HzrInit();

	ServiceData.ServiceStatus.dwCurrentState = SERVICE_RUNNING;
	SetServiceStatus(ServiceData.ServiceStatusHandle, &ServiceData.ServiceStatus);
}

DWORD WINAPI HzrHandlerEx(
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

		HzrServiceCleanup();

		ServiceData.ServiceStatus.dwCurrentState = SERVICE_STOPPED;
		SetServiceStatus(ServiceData.ServiceStatusHandle, &ServiceData.ServiceStatus);

		return NO_ERROR;
	}

	case SERVICE_CONTROL_INTERROGATE:
		return NO_ERROR;
	}

	return ERROR_CALL_NOT_IMPLEMENTED;
}