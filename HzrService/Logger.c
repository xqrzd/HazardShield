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

#include "Logger.h"
#include <stdio.h>

#define LOG_FILE L"%AllUsersProfile%\\Hazard Shield\\rts_log.txt"
#define LOG_MAX_ENTRY_SIZE 1024

BOOLEAN LogMessageA(
	_In_ CONST PCHAR Format,
	...)
{
	BOOLEAN success = FALSE;
	CHAR buffer[LOG_MAX_ENTRY_SIZE];
	WCHAR logFilePath[MAX_PATH];
	INT length;
	va_list argptr;
	SYSTEMTIME time;

	va_start(argptr, Format);
	GetLocalTime(&time);

	// Write current time.
	length = sprintf_s(buffer, sizeof(buffer), "%d/%d/%d %02d:%02d - ", time.wMonth, time.wDay, time.wYear, time.wHour, time.wMinute);
	// Write data (save room for new line).
	length += vsprintf_s(buffer + length, sizeof(buffer) - length - 2, Format, argptr);

	// Append new line and increase length appropriately
	buffer[length] = '\r';
	buffer[length + 1] = '\n';
	length += 2;

	if (ExpandEnvironmentStringsW(LOG_FILE, logFilePath, sizeof(logFilePath) / sizeof(WCHAR)))
	{
		HANDLE fileHandle;
		DWORD creationDisposition;

		if (GetFileAttributesW(logFilePath) == INVALID_FILE_ATTRIBUTES)
			creationDisposition = CREATE_NEW;
		else
			creationDisposition = OPEN_EXISTING;

		fileHandle = CreateFileW(logFilePath, FILE_APPEND_DATA, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, creationDisposition, FILE_ATTRIBUTE_NORMAL, NULL);

		if (fileHandle != INVALID_HANDLE_VALUE)
		{
			DWORD bytesWritten;

			success = (BOOLEAN)WriteFile(fileHandle, buffer, length, &bytesWritten, NULL);

			CloseHandle(fileHandle);
		}
	}

	va_end(argptr);

	return success;
}