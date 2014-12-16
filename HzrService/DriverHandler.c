/*
*  Copyright (C) 2014 Orbitech
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

#include "DriverHandler.h"
#include "Logger.h"
#include <fltUser.h> // Relies on Windows.h

#define DRV_CMD_GET_BUFFER 1

#define OP_SCAN_FILE 1
#define OP_REG_VALUE_CHANGED 2

typedef struct _DRIVER_REQUEST {
	ULONG Handle;
	ULONG BufferSize;
} DRIVER_REQUEST, *PDRIVER_REQUEST;

typedef struct _DRIVER_MESSAGE {
	FILTER_MESSAGE_HEADER MessageHeader;
	DRIVER_REQUEST Request;
	OVERLAPPED Overlapped;
} DRIVER_MESSAGE, *PDRIVER_MESSAGE;

typedef struct _SERVICE_REQUEST_BUFFER {
	ULONG Command;
	ULONG Handle;
} SERVICE_REQUEST_BUFFER, *PSERVICE_REQUEST_BUFFER;

typedef struct _SERVICE_RESPONSE {
	FILTER_REPLY_HEADER ReplyHeader;
	BOOLEAN Flags;
} SERVICE_RESPONSE, *PSERVICE_RESPONSE;

typedef struct _FILE_SCAN_INFO {
	UCHAR OperationType;
	UCHAR FileAccess;
	USHORT FileNameLength;
	ULONG FileNameOffset;
	ULONG FileSize;
	ULONG FileDataOffset;
} FILE_SCAN_INFO, *PFILE_SCAN_INFO;

HRESULT DrvpGetBuffer(
	_In_ PDRIVER_INSTANCE DriverInstance,
	_In_ PDRIVER_REQUEST Request,
	_Out_ PUCHAR Buffer)
{
	DWORD bytesReturned;
	SERVICE_REQUEST_BUFFER serviceRequest;

	serviceRequest.Command = DRV_CMD_GET_BUFFER;
	serviceRequest.Handle = Request->Handle;

	return FilterSendMessage(
		DriverInstance->CommunicationPort,
		&serviceRequest,
		sizeof(serviceRequest),
		Buffer,
		Request->BufferSize,
		&bytesReturned);

	// TODO: Check that bytesReturned == Request->BufferSize
}

BOOLEAN DrvpReply(
	_In_ PDRIVER_INSTANCE DriverInstance,
	_In_ PFILTER_MESSAGE_HEADER MessageHeader,
	_In_ BOOLEAN Flags)
{
	HRESULT result;
	SERVICE_RESPONSE response;

	response.ReplyHeader.MessageId = MessageHeader->MessageId;
	response.ReplyHeader.Status = 0;
	response.Flags = Flags;

	result = FilterReplyMessage(DriverInstance->CommunicationPort, &response.ReplyHeader, MessageHeader->ReplyLength);

	if (result != S_OK)
		LogMessageA("DriverHandler::DrvpReply FilterReplyMessage failed %X", result);

	return result == S_OK;
}

VOID DrvpMessageReceived(
	_In_ PDRIVER_INSTANCE DriverInstance,
	_In_ PDRIVER_MESSAGE Message)
{
	BOOLEAN flags = 0;
	PUCHAR buffer = malloc(Message->Request.BufferSize);

	if (buffer)
	{
		UCHAR operation;

		DrvpGetBuffer(DriverInstance, &Message->Request, buffer);

		operation = buffer[0];

		switch (operation)
		{
		case OP_SCAN_FILE:
		{
			FILE_INFO fileInfo;
			PFILE_SCAN_INFO scanInfoDrv = (PFILE_SCAN_INFO)buffer;

			fileInfo.FileAccess = scanInfoDrv->FileAccess;
			fileInfo.FileName = (PWCHAR)(buffer + scanInfoDrv->FileNameOffset);
			fileInfo.FileNameLength = scanInfoDrv->FileNameLength;
			fileInfo.FileData = buffer + scanInfoDrv->FileDataOffset;
			fileInfo.FileSize = scanInfoDrv->FileSize;

			flags = DriverInstance->FileCallback(&fileInfo);

			break;
		}
		}

		free(buffer);
	}

	DrvpReply(DriverInstance, &Message->MessageHeader, flags);
}

DWORD WINAPI DrvpEventHandler(LPVOID DriverInstance)
{
	PDRIVER_INSTANCE driverInstance = (PDRIVER_INSTANCE)DriverInstance;
	DRIVER_MESSAGE messageBuffer;

	ZeroMemory(&messageBuffer.Overlapped, sizeof(OVERLAPPED));

	HRESULT result = FilterGetMessage(
		driverInstance->CommunicationPort,
		&messageBuffer.MessageHeader,
		FIELD_OFFSET(DRIVER_MESSAGE, Overlapped),
		&messageBuffer.Overlapped);

	if (result != HRESULT_FROM_WIN32(ERROR_IO_PENDING))
		LogMessageA("DrvpEventHandler::FilterGetMessage failed %X", result);

	while (TRUE)
	{
		HRESULT result;
		BOOL success;
		DWORD outSize;
		ULONG_PTR key;
		LPOVERLAPPED overlapped;
		PDRIVER_MESSAGE message;

		success = GetQueuedCompletionStatus(driverInstance->CompletionPort, &outSize, &key, &overlapped, INFINITE);

		if (!success)
			break;

		// Obtain the message. Note that this message may not be the same as messageBuffer, since there are multiple threads.
		message = CONTAINING_RECORD(overlapped, DRIVER_MESSAGE, Overlapped);

		DrvpMessageReceived(driverInstance, message);

		result = FilterGetMessage(
			driverInstance->CommunicationPort,
			&message->MessageHeader,
			FIELD_OFFSET(DRIVER_MESSAGE, Overlapped),
			&message->Overlapped);

		if (result != HRESULT_FROM_WIN32(ERROR_IO_PENDING))
			break;
	}

	return 0;
}

BOOLEAN DrvConnect(
	_In_ LPCWSTR PortName,
	_In_ DWORD NumberOfScanThreads,
	_In_ PFILE_CALLBACK FileCallback,
	_Out_ PDRIVER_INSTANCE DriverInstance)
{
	HRESULT result;
	HANDLE communicationPort;
	HANDLE completionPort;

	result = FilterConnectCommunicationPort(PortName, 0, NULL, 0, NULL, &communicationPort);

	if (result == S_OK)
	{
		completionPort = CreateIoCompletionPort(communicationPort, NULL, 0, NumberOfScanThreads);

		if (completionPort)
		{
			DriverInstance->CommunicationPort = communicationPort;
			DriverInstance->CompletionPort = completionPort;
			DriverInstance->NumberOfScanThreads = NumberOfScanThreads;
			DriverInstance->FileCallback = FileCallback;
		}
		else
		{
			DWORD error = GetLastError();
			CloseHandle(communicationPort);
			result = HRESULT_FROM_WIN32(error);
			LogMessageA("DrvConnect::CreateIoCompletionPort failed %u", error);
		}
	}
	else
		LogMessageA(":DrvConnect::FilterConnectCommunicationPort failed %X", result);

	return result == S_OK;
}

VOID DrvStartEventMonitor(
	_In_ PDRIVER_INSTANCE DriverInstance)
{
	for (DWORD i = 0; i < DriverInstance->NumberOfScanThreads; i++)
	{
		HANDLE threadHandle = CreateThread(NULL, 0, DrvpEventHandler, DriverInstance, 0, NULL);
		CloseHandle(threadHandle);
	}
}