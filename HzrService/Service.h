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

#define SERVICE_NAME L"HazardShield"
#define FILTER_NAME L"HzrFilter"
#define PORT_NAME L"\\HzrFilterPort"

#define CLAMAV_DATABASE_PATH "%AllUsersProfile%\\Hazard Shield"

VOID WINAPI HzrServiceMain(
	_In_ DWORD dwArgc,
	_In_ LPTSTR *lpszArgv
	);

DWORD WINAPI HzrHandlerEx(
	_In_ DWORD dwControl,
	_In_ DWORD dwEventType,
	_In_ LPVOID lpEventData,
	_In_ LPVOID lpContext
	);