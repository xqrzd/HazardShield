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

typedef enum _HS_SCAN_REASON {
	HsScanOnPeOpen
} HS_SCAN_REASON;

typedef struct _HS_SCANNER_NOTIFICATION {
	HS_SCAN_REASON ScanReason;
	USHORT FileNameLength;
	LONGLONG ScanId;
} HS_SCANNER_NOTIFICATION, *PHS_SCANNER_NOTIFICATION;

typedef struct _HS_DRIVER_NOTIFICATION {
	FILTER_MESSAGE_HEADER MessageHeader;
	HS_SCANNER_NOTIFICATION Notification;
	OVERLAPPED Overlapped;
} HS_DRIVER_NOTIFICATION, *PHS_DRIVER_NOTIFICATION;

typedef enum _HS_COMMAND {
	HsCmdCreateSectionForDataScan,
	HsCmdQueryFileName
} HS_COMMAND;

typedef struct _HS_SERVICE_RESPONSE {
	FILTER_REPLY_HEADER ReplyHeader;
	BOOLEAN Flags;
} HS_SERVICE_RESPONSE, *PHS_SERVICE_RESPONSE;

NTSTATUS NTAPI KhspUserScanWorker(
	_In_ PVOID Parameter
	);

NTSTATUS KhspHandleScanMessage(
	_In_ PHS_SCANNER_NOTIFICATION Notification,
	_Out_ PUCHAR ResponseFlags
	);

NTSTATUS KhspHandleScanPeOpen(
	_In_ LONGLONG ScanId,
	_In_ USHORT FileNameLength,
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

PPH_STRING KhspQueryFileName(
	_In_ LONGLONG ScanId,
	_In_ USHORT FileNameLength
	);