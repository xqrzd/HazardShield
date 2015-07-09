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
#include "Utility.h"
#include <Sfc.h>

PPH_BYTES HspGetClamAvDatabaseDirectory(
	);

struct {
	SERVICE_STATUS ServiceStatus;
	SERVICE_STATUS_HANDLE ServiceStatusHandle;
	PH_QUEUED_LOCK ScannerLock;
	PHS_SCANNER Scanner;
} ServiceData;

UCHAR ScanRoutine(
	_In_ LONGLONG ScanId,
	_In_ PPH_STRING FileName)
{
	HS_FILE_DATA fileData;
	cl_error_t result;
	PPH_BYTES virusName;

	// Skip scanning files protected by Windows.

	if (SfcIsFileProtected(NULL, FileName->Buffer))
	{
		return HS_RESPONSE_FLAG_CLEAN;
	}

	if (!NT_SUCCESS(KhsReadFile(ScanId, &fileData)))
	{
		wprintf(L"KhsReadFile failed for %s\n", FileName->Buffer);

		return HS_RESPONSE_FLAG_CLEAN;
	}

	wprintf(L"Scanning %s\n", FileName->Buffer);

	// Acquire scanner lock in shared access. This is done
	// so the database can be updated without having to
	// stop filtering.

	PhAcquireQueuedLockShared(&ServiceData.ScannerLock);

	result = HsScanBuffer(
		ServiceData.Scanner,
		fileData.BaseAddress,
		fileData.Size,
		CL_SCAN_STDOPT,
		&virusName);

	PhReleaseQueuedLockShared(&ServiceData.ScannerLock);

	KhsCloseFile(&fileData);

	if (result == CL_VIRUS)
	{
		printf("Found virus: %s\n", virusName->Buffer);

		PhDereferenceObject(virusName);

		return HS_RESPONSE_FLAG_INFECTED;
	}

	return HS_RESPONSE_FLAG_CLEAN;
}

cl_error_t HspLoadClamAV()
{
	cl_error_t result;
	PPH_BYTES databaseDirectory;

	if (!HsCreateScanner(&ServiceData.Scanner))
		return CL_EMEM;

	if (!(databaseDirectory = HspGetClamAvDatabaseDirectory()))
		return CL_EMEM;

	result = HsLoadClamAvDatabase(
		ServiceData.Scanner,
		databaseDirectory->Buffer,
		CL_DB_BYTECODE);

	if (result == CL_SUCCESS)
	{
		result = HsCompileClamAvDatabase(ServiceData.Scanner);
	}
	else
		printf("HsLoadClamAvDatabase failed %d\n", result);

	PhDereferenceObject(databaseDirectory);

	return result;
}

VOID HspServiceInit()
{
	HRESULT result = KhsConnect(KHS_PORT_NAME);

	if (SUCCEEDED(result))
	{
		HsInit();
		HspLoadClamAV();

		printf("Sigs loaded: %u\n", ServiceData.Scanner->Signatures);

		PhInitializeQueuedLock(&ServiceData.ScannerLock);

		// Finished service initialization. Start listening
		// for messages from the driver.

		KhsStartFiltering(HsGetProcessorCount(), ScanRoutine);
	}
	else
		printf("KhsConnect failed %X\n", result);
}

VOID HspServiceCleanup()
{
	KhsDisconnect();

	HsDeleteScanner(ServiceData.Scanner);
}

PPH_BYTES HspGetClamAvDatabaseDirectory()
{
	static PH_STRINGREF path = PH_STRINGREF_INIT(CLAMAV_DATABASE_PATH);

	PPH_STRING expandedPathUnicode;
	PPH_BYTES expandedPathUtf8 = NULL;

	expandedPathUnicode = PhExpandEnvironmentStrings(&path);

	if (expandedPathUnicode)
	{
		// cl_load expects UTF-8 format.
		expandedPathUtf8 = PhConvertUtf16ToUtf8(expandedPathUnicode->Buffer);

		PhDereferenceObject(expandedPathUnicode);
	}

	return expandedPathUtf8;
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

		HspServiceCleanup();

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