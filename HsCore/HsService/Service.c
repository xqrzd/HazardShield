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
#include "KhsUser.h"
#include "Scanner.h"
#include <Sfc.h>

struct {
	SERVICE_STATUS ServiceStatus;
	SERVICE_STATUS_HANDLE ServiceStatusHandle;
	SRWLOCK ScannerLock;
	PHS_SCANNER Scanner;
} ServiceData;

UCHAR ScanRoutine(
	_In_ PHS_FILE_INFO FileInfo)
{
	cl_error_t result;
	PCHAR virusName;
	PPH_BYTES virusNameCopy;

	// Skip scanning files protected by Windows.

	if (SfcIsFileProtected(NULL, FileInfo->FilePath))
	{
		return HS_RESPONSE_FLAG_CLEAN;
	}

	wprintf(L"Scanning %s\n", FileInfo->FilePath);

	// Acquire scanner lock in shared access. This is done
	// so the database can be updated without having to
	// stop filtering.

	AcquireSRWLockShared(&ServiceData.ScannerLock);

	result = HsScanBuffer(
		ServiceData.Scanner,
		FileInfo->Buffer,
		FileInfo->BufferSize,
		CL_SCAN_STDOPT,
		&virusName);

	if (result == CL_VIRUS)
	{
		// Make a copy of virusName. virusName is used outside
		// of the lock, so it could become invalid if the
		// database was reloaded.

		virusNameCopy = PhCreateBytes(virusName);
	}

	ReleaseSRWLockShared(&ServiceData.ScannerLock);

	if (result == CL_VIRUS)
	{
		printf("Found virus: %s\n", virusNameCopy->Buffer);

		PhDereferenceObject(virusNameCopy);

		return HS_RESPONSE_FLAG_INFECTED;
	}

	return HS_RESPONSE_FLAG_CLEAN;
}

VOID HspLoadClamAV()
{
	HsCreateScanner(&ServiceData.Scanner);
	HsLoadClamAvDatabase(ServiceData.Scanner, "C:\\ProgramData\\Hazard Shield", CL_DB_BYTECODE);
	HsCompileClamAvDatabase(ServiceData.Scanner);
}

VOID HspServiceInit()
{
	HRESULT result = KhsConnect(KHS_PORT_NAME);

	if (SUCCEEDED(result))
	{
		SYSTEM_INFO systemInfo;

		HsInit();
		HspLoadClamAV();
		GetSystemInfo(&systemInfo);

		printf("Sigs loaded: %u\n", ServiceData.Scanner->Signatures);

		InitializeSRWLock(&ServiceData.ScannerLock);

		// All service initialization is done, start listening
		// for messages from the driver.

		KhsStartFiltering(systemInfo.dwNumberOfProcessors, ScanRoutine);
	}
	else
		printf("KhsConnect failed %X\n", result);
}

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

	HspServiceInit();

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

//void __cdecl main()
//{
//	HspServiceInit();
//	getchar();
//}