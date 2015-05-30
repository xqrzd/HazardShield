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
	_In_ PHS_DRIVER_NOTIFICATION Notification
	);

HRESULT KhspFilterReplyMessage(
	_In_ PFILTER_MESSAGE_HEADER MessageHeader,
	_In_ BOOLEAN Flags
	);

HRESULT KhspCreateSectionForDataScan(
	_In_ LONGLONG ScanId,
	_Out_ PHANDLE SectionHandle
	);

HANDLE HsKhsPortHandle;
HANDLE HsKhsCompletionPort;

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
	_In_ DWORD NumberOfScanThreads)
{
	if (!HsKhsPortHandle)
		return STATUS_INVALID_HANDLE;

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
		BOOL success;
		DWORD outSize;
		ULONG_PTR key;
		LPOVERLAPPED overlapped;

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

		KhspHandleScanMessage(notification);
	}

	return STATUS_SUCCESS;
}

NTSTATUS KhspHandleScanMessage(
	_In_ PHS_DRIVER_NOTIFICATION Notification)
{
	HRESULT result;
	HANDLE sectionHandle;

	result = KhspCreateSectionForDataScan(
		Notification->Notification.ScanId,
		&sectionHandle);

	if (SUCCEEDED(result))
	{
		PVOID address;

		address = MapViewOfFile(
			sectionHandle,
			FILE_MAP_READ,
			0,
			0,
			0);

		if (address)
		{
			UnmapViewOfFile(address);
		}

		CloseHandle(sectionHandle);
	}

	KhspFilterReplyMessage(&Notification->MessageHeader, 0);

	return STATUS_SUCCESS;
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