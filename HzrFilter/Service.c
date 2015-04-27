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

#define SERVICE_BUFFER_TAG 'vSzH'

/// <summary>
/// Sends data using FltSendMessage.
/// </summary>
/// <param name="HandleSystem">A pointer to a HANDLE_SYSTEM.</param>
/// <param name="Filter">A pointer to the filter instance.</param>
/// <param name="ClientPort">A pointer to the client communication handle.</param>
/// <param name="BufferInfo">The buffer to send.</param>
/// <param name="Response">The response from the user-application.</param>
NTSTATUS HspSendMessage(
	_In_ PHANDLE_SYSTEM HandleSystem,
	_In_ PFLT_FILTER Filter,
	_In_ PFLT_PORT* ClientPort,
	_In_ PBUFFER_INFO BufferInfo,
	_Out_ PSERVICE_RESPONSE Response
	);

NTSTATUS HsScanFileUserMode(
	_In_ PHANDLE_SYSTEM HandleSystem,
	_In_ PFLT_FILTER Filter,
	_In_ PFLT_PORT* ClientPort,
	_In_ UCHAR FileAccess,
	_In_ PUNICODE_STRING FilePath,
	_In_ PVOID Buffer,
	_In_ ULONG BufferSize,
	_Out_ PSERVICE_RESPONSE Response)
{
	NTSTATUS status;
	BUFFER_INFO bufferInfo;

	// Allocate space for the FILE_SCAN_INFO header, the file data, file path and a NULL terminator.
	bufferInfo.BufferSize = sizeof(FILE_SCAN_INFO) + BufferSize + FilePath->Length + sizeof(WCHAR);
	bufferInfo.Buffer = ExAllocatePoolWithTag(PagedPool, bufferInfo.BufferSize, SERVICE_BUFFER_TAG);

	if (bufferInfo.Buffer)
	{
		ULONG offset;
		PFILE_SCAN_INFO info;

		offset = sizeof(FILE_SCAN_INFO);
		info = bufferInfo.Buffer;

		info->OperationType = OP_SCAN_FILE;
		info->FileAccess = FileAccess;
		info->FileNameLength = FilePath->Length;
		info->FileSize = BufferSize;

		info->FileDataOffset = offset;
		WriteData(bufferInfo.Buffer, Buffer, BufferSize, offset);

		info->FileNameOffset = offset;
		WriteData(bufferInfo.Buffer, FilePath->Buffer, FilePath->Length, offset);

		// Add NULL terminator to file path.
		*(PWCHAR)((PUCHAR)bufferInfo.Buffer + offset) = L'\0';

		status = HspSendMessage(HandleSystem, Filter, ClientPort, &bufferInfo, Response);

		ExFreePoolWithTag(bufferInfo.Buffer, SERVICE_BUFFER_TAG);
	}
	else
		status = STATUS_INSUFFICIENT_RESOURCES;

	return status;
}

NTSTATUS HspSendMessage(
	_In_ PHANDLE_SYSTEM HandleSystem,
	_In_ PFLT_FILTER Filter,
	_In_ PFLT_PORT* ClientPort,
	_In_ PBUFFER_INFO BufferInfo,
	_Out_ PSERVICE_RESPONSE Response)
{
	NTSTATUS status;
	ULONG handle;

	// Create a handle for the client to access the buffer contents.
	status = HndCreateHandle(HandleSystem, BufferInfo, &handle);

	if (NT_SUCCESS(status))
	{
		DRIVER_REQUEST request;
		ULONG replyLength;

		request.Handle = handle;
		request.BufferSize = BufferInfo->BufferSize;
		replyLength = sizeof(SERVICE_RESPONSE);

		status = FltSendMessage(
			Filter,
			ClientPort,
			&request,
			sizeof(request),
			Response,
			&replyLength,
			NULL);

		// FltSendMessage waits for a reply, so by this point
		// the user-app will be done with the handle.
		HndReleaseHandle(HandleSystem, handle);
	}

	return status;
}
