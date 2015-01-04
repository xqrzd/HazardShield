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

#pragma once

#include <Windows.h>

#define FILE_ACCESS_READ 0x1
#define FILE_ACCESS_WRITE 0x2
#define FILE_ACCESS_EXECUTE 0x4

#define RESPONSE_FLAG_BLOCK_OPERATION 0x1
#define RESPONSE_FLAG_DELETE 0x2
#define RESPONSE_FLAG_WHITELIST 0x4

typedef struct _FILE_INFO {
	UCHAR FileAccess;
	USHORT FileNameLength;
	PWCHAR FileName;
	ULONG FileSize;
	PVOID FileData;
} FILE_INFO, *PFILE_INFO;

typedef BOOLEAN FILE_CALLBACK(
	_In_ PFILE_INFO FileInfo
	);
typedef FILE_CALLBACK *PFILE_CALLBACK;

typedef struct _DRIVER_INSTANCE {
	HANDLE CommunicationPort;
	HANDLE CompletionPort;
	DWORD NumberOfScanThreads;

	PFILE_CALLBACK FileCallback;
} DRIVER_INSTANCE, *PDRIVER_INSTANCE;

BOOLEAN DrvConnect(
	_In_ LPCWSTR PortName,
	_In_ DWORD NumberOfScanThreads,
	_In_ PFILE_CALLBACK FileCallback,
	_Out_ PDRIVER_INSTANCE DriverInstance
	);

VOID DrvStartEventMonitor(
	_In_ PDRIVER_INSTANCE DriverInstance
	);