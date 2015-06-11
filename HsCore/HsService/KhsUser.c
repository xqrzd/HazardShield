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

#include "KhsUser.h"
#include <fltUser.h>
#include "KhspUser.h"

NTSTATUS NTAPI KhspUserScanWorker(
	_In_ PVOID Parameter
	);

NTSTATUS KhspHandleScanMessage(
	_In_ PHS_SCANNER_NOTIFICATION Notification,
	_Out_ PUCHAR ResponseFlags
	);

NTSTATUS KhspHandleScanPeOpen(
	_In_ LONGLONG ScanId,
	_Out_ PUCHAR ResponseFlags
	);

HRESULT KhspFilterReplyMessage(
	_In_ PFILTER_MESSAGE_HEADER MessageHeader,
	_In_ BOOLEAN Flags
	);

HRESULT KhspCreateSectionForDataScan(
	_In_ LONGLONG ScanId,
	_Out_ PHANDLE SectionHandle
	);

HRESULT KhspQueryFileName(
	_In_ LONGLONG ScanId,
	_In_ USHORT FileNameSizeInBytes,
	_Out_ PWSTR FileName
	);

HANDLE HsKhsPortHandle;
HANDLE HsKhsCompletionPort;

PHS_SCAN_FILE_ROUTINE HsFileScanRoutine;

HRESULT KhsConnect(
	_In_ LPCWSTR PortName)
{
	HRESULT result;
	HANDLE communicationPort;

	result = FilterConnectCommunicationPort(
		PortName,
		0,
		NULL,
		0,
		NULL,
		&communicationPort);

	if (SUCCEEDED(result))
	{
		HsKhsPortHandle = communicationPort;
	}

	return result;
}

NTSTATUS KhsStartFiltering(
	_In_ DWORD NumberOfScanThreads,
	_In_ PHS_SCAN_FILE_ROUTINE FileScanRoutine)
{
	if (!HsKhsPortHandle)
		return STATUS_INVALID_HANDLE;

	HsFileScanRoutine = FileScanRoutine;

	HsKhsCompletionPort = CreateIoCompletionPort(
		HsKhsPortHandle,
		NULL,
		0,
		NumberOfScanThreads);

	if (HsKhsCompletionPort)
	{
		for (DWORD i = 0; i < NumberOfScanThreads; i++)
		{
			HANDLE threadHandle = CreateThread(
				NULL,
				0,
				KhspUserScanWorker,
				NULL,
				0,
				NULL);

			CloseHandle(threadHandle);
		}

		return STATUS_SUCCESS;
	}
	else
		return NTSTATUS_FROM_WIN32(GetLastError());
}

VOID KhsDisconnect()
{
	CloseHandle(HsKhsPortHandle);
	CloseHandle(HsKhsCompletionPort);

	HsKhsPortHandle = NULL;
	HsKhsCompletionPort = NULL;

	HsFileScanRoutine = NULL;
}

NTSTATUS NTAPI KhspUserScanWorker(
	_In_ PVOID Parameter)
{
	HS_DRIVER_NOTIFICATION notificationBuffer;
	PHS_DRIVER_NOTIFICATION notification;

	RtlZeroMemory(&notificationBuffer.Overlapped, sizeof(OVERLAPPED));
	notification = &notificationBuffer;

	while (TRUE)
	{
		HRESULT result;
		NTSTATUS status;
		BOOL success;
		DWORD outSize;
		ULONG_PTR key;
		LPOVERLAPPED overlapped;
		UCHAR responseFlags;

		result = FilterGetMessage(
			HsKhsPortHandle,
			&notification->MessageHeader,
			FIELD_OFFSET(HS_DRIVER_NOTIFICATION, Overlapped),
			&notification->Overlapped);

		if (result != HRESULT_FROM_WIN32(ERROR_IO_PENDING))
			break;

		success = GetQueuedCompletionStatus(
			HsKhsCompletionPort,
			&outSize,
			&key,
			&overlapped,
			INFINITE);

		if (!success)
			break;

		// Obtain the notification. Note that this notification may not be
		// the same as messageBuffer, since there are multiple threads.

		notification = CONTAINING_RECORD(overlapped, HS_DRIVER_NOTIFICATION, Overlapped);

		status = KhspHandleScanMessage(
			&notification->Notification,
			&responseFlags);

		if (!NT_SUCCESS(status))
		{
			// If the callback fails, return no flags to the driver,
			// which will allow the operation to pass through.

			responseFlags = 0;
		}

		KhspFilterReplyMessage(&notification->MessageHeader, responseFlags);
	}

	return STATUS_SUCCESS;
}

NTSTATUS KhspHandleScanMessage(
	_In_ PHS_SCANNER_NOTIFICATION Notification,
	_Out_ PUCHAR ResponseFlags)
{
	switch (Notification->ScanReason)
	{
	case HsScanOnPeOpen:
		return KhspHandleScanPeOpen(Notification->ScanId, ResponseFlags);
	}

	return STATUS_INVALID_PARAMETER;
}

NTSTATUS KhspHandleScanPeOpen(
	_In_ LONGLONG ScanId,
	_Out_ PUCHAR ResponseFlags)
{
	NTSTATUS status = STATUS_UNSUCCESSFUL;
	HRESULT result;
	HANDLE sectionHandle;

	if (!HsFileScanRoutine)
		return STATUS_NO_CALLBACK_ACTIVE;

	result = KhspCreateSectionForDataScan(ScanId, &sectionHandle);

	if (!SUCCEEDED(result))
		return status;

	PVOID address;

	address = MapViewOfFile(
		sectionHandle,
		FILE_MAP_READ,
		0,
		0,
		0);

	if (address)
	{
		MEMORY_BASIC_INFORMATION basicInfo;

		if (VirtualQuery(address, &basicInfo, sizeof(basicInfo)))
		{
			HS_FILE_INFO fileInfo;
			WCHAR filePath[MAX_PATH];

			fileInfo.Buffer = address;
			fileInfo.BufferSize = basicInfo.RegionSize;

			if (SUCCEEDED(KhspQueryFileName(ScanId, sizeof(filePath), filePath)))
			{
				fileInfo.FilePath = filePath;
			}
			else
				fileInfo.FilePath = NULL;

			status = HsFileScanRoutine(&fileInfo, ResponseFlags);
		}

		UnmapViewOfFile(address);
	}

	CloseHandle(sectionHandle);

	return status;
}

HRESULT KhspFilterReplyMessage(
	_In_ PFILTER_MESSAGE_HEADER MessageHeader,
	_In_ BOOLEAN Flags)
{
	HS_SERVICE_RESPONSE response;

	response.ReplyHeader.MessageId = MessageHeader->MessageId;
	response.ReplyHeader.Status = STATUS_SUCCESS;
	response.Flags = Flags;

	return FilterReplyMessage(
		HsKhsPortHandle,
		&response.ReplyHeader,
		MessageHeader->ReplyLength);
}

HRESULT KhspCreateSectionForDataScan(
	_In_ LONGLONG ScanId,
	_Out_ PHANDLE SectionHandle)
{
	HRESULT result;
	HANDLE sectionHandle;
	DWORD bytesReturned;

	struct
	{
		ULONG Command;
		LONGLONG ScanId;
	} input = { HsCmdCreateSectionForDataScan, ScanId };

	result = FilterSendMessage(
		HsKhsPortHandle,
		&input,
		sizeof(input),
		&sectionHandle,
		sizeof(HANDLE),
		&bytesReturned);

	if (SUCCEEDED(result))
		*SectionHandle = sectionHandle;

	return result;
}

HRESULT KhspQueryFileName(
	_In_ LONGLONG ScanId,
	_In_ USHORT FileNameSizeInBytes,
	_Out_ PWSTR FileName)
{
	HRESULT result;
	DWORD bytesReturned;

	struct
	{
		ULONG Command;
		LONGLONG ScanId;
	} input = { HsCmdQueryFileName, ScanId };

	result = FilterSendMessage(
		HsKhsPortHandle,
		&input,
		sizeof(input),
		FileName,
		FileNameSizeInBytes - sizeof(WCHAR),
		&bytesReturned);

	if (SUCCEEDED(result))
	{
		// Add NULL terminator to file name.
		FileName[bytesReturned / sizeof(WCHAR)] = L'\0';
	}

	return result;
}