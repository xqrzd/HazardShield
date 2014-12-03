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

#include "Service.h"

NTSTATUS SvcpSendMessage(
	_In_ PHANDLE_SYSTEM HandleSystem,
	_In_ PFLT_FILTER Filter,
	_In_ PFLT_PORT* ClientPort,
	_In_ PBUFFER_INFO BufferInfo,
	_Out_ PSERVICE_RESPONSE Response)
{
	NTSTATUS status;
	ULONG handle;

	// Create a handle for the client to send back to the driver, in order to retrieve the buffer contents
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

		// The handle isn't needed after FltSendMessage returns, since the client will have responded
		HndReleaseHandle(HandleSystem, handle);
	}

	return status;
}

NTSTATUS SvcScanFile(
	_In_ PHANDLE_SYSTEM HandleSystem,
	_In_ PFLT_FILTER Filter,
	_In_ PFLT_PORT* ClientPort,
	_In_ UCHAR FileAccess,
	_In_ POBJECT_NAME_INFORMATION FullFilePath,
	_In_ PVOID FileData,
	_In_ ULONG FileSize,
	_Out_ PSERVICE_RESPONSE Response)
{
	NTSTATUS status;
	BUFFER_INFO bufferInfo;

	bufferInfo.BufferSize = sizeof(FILE_SCAN_INFO) + FullFilePath->Name.Length + FileSize;
	bufferInfo.Buffer = ExAllocatePoolWithTag(PagedPool, bufferInfo.BufferSize, 'fibf');

	if (bufferInfo.Buffer)
	{
		ULONG offset = sizeof(FILE_SCAN_INFO);
		PFILE_SCAN_INFO info = bufferInfo.Buffer;

		info->OperationType = OP_SCAN_FILE;
		info->FileAccess = FileAccess;
		info->FileNameLength = FullFilePath->Name.Length;
		info->FileSize = FileSize;

		info->FileDataOffset = offset;
		WriteData(bufferInfo.Buffer, FileData, FileSize, offset);

		info->FileNameOffset = offset;
		WriteData(bufferInfo.Buffer, FullFilePath->Name.Buffer, FullFilePath->Name.Length, offset);

		status = SvcpSendMessage(HandleSystem, Filter, ClientPort, &bufferInfo, Response);

		ExFreePoolWithTag(bufferInfo.Buffer, 'fibf');
	}
	else
		status = STATUS_INSUFFICIENT_RESOURCES;

	return status;
}