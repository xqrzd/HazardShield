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

#ifndef HZRFILTER_SERVICE_H
#define HZRFILTER_SERVICE_H

#include <fltKernel.h>
#include "Handle.h"

#define DRV_CMD_GET_BUFFER 1
#define DRV_CMD_PROTECT_PROCESS 2

#define OP_SCAN_FILE 1
#define OP_REG_VALUE_CHANGED 2

#define RESPONSE_FLAG_BLOCK_OPERATION 0x1
#define RESPONSE_FLAG_DELETE 0x2
#define RESPONSE_FLAG_WHITELIST 0x4

#define FILE_ACCESS_READ 0x1
#define FILE_ACCESS_WRITE 0x2
#define FILE_ACCESS_EXECUTE 0x4

// Writes Source into Destination, and increments Offset by Length.
#define WriteData(Destination, Source, Length, Offset) { \
	RtlCopyMemory((PUCHAR)Destination + Offset, Source, Length); \
	Offset += Length; \
}

typedef struct _BUFFER_INFO {
	PVOID Buffer;
	ULONG BufferSize;
} BUFFER_INFO, *PBUFFER_INFO;

typedef struct _FILE_SCAN_INFO {
	UCHAR OperationType;
	UCHAR FileAccess;
	USHORT FileNameLength;
	ULONG FileNameOffset;
	ULONG FileSize;
	ULONG FileDataOffset;
} FILE_SCAN_INFO, *PFILE_SCAN_INFO;

typedef struct _DRIVER_REQUEST {
	ULONG Handle;
	ULONG BufferSize;
} DRIVER_REQUEST, *PDRIVER_REQUEST;

typedef struct _SERVICE_RESPONSE {
	BOOLEAN Flags;
} SERVICE_RESPONSE, *PSERVICE_RESPONSE;

typedef struct _SERVICE_REQUEST_BUFFER {
	ULONG Command;
	ULONG Handle;
} SERVICE_REQUEST_BUFFER, *PSERVICE_REQUEST_BUFFER;

typedef struct _SERVICE_REQUEST_PROTECT_PROCESS {
	ULONG Command;
	ULONG ProcessId;
	ACCESS_MASK AccessBitsToClear;
} SERVICE_REQUEST_PROTECT_PROCESS, *PSERVICE_REQUEST_PROTECT_PROCESS;

NTSTATUS SvcScanFile(
	_In_ PHANDLE_SYSTEM HandleSystem,
	_In_ PFLT_FILTER Filter,
	_In_ PFLT_PORT* ClientPort,
	_In_ UCHAR FileAccess,
	_In_ POBJECT_NAME_INFORMATION FullFilePath,
	_In_ PVOID FileData,
	_In_ ULONG FileSize,
	_Out_ PSERVICE_RESPONSE Response
	);

#endif