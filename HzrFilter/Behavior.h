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

#ifndef HZRFILTER_BEHAVIOR_H
#define HZRFILTER_BEHAVIOR_H

#include <fltKernel.h>

typedef enum _HS_PROCESS_TYPE {
	HS_PROCESS_TYPE_NONE = 0,
	HS_PROCESS_TYPE_FIREFOX_PLUGIN_HOST
} HS_PROCESS_TYPE, *PHS_PROCESS_TYPE;

typedef struct _HS_MONITORED_PROCESS {
	PEPROCESS Process;
	HS_PROCESS_TYPE Type;
} HS_MONITORED_PROCESS, *PHS_MONITORED_PROCESS;

VOID HsInitializeBehaviorSystem(
	);

VOID HsDeleteBehaviorSystem(
	);

BOOLEAN HsMonitorPreWrite(
	_Inout_ PFLT_CALLBACK_DATA Data,
	_In_ PCFLT_RELATED_OBJECTS FltObjects
	);

VOID HsMonitorCreateProcessNotifyEx(
	_Inout_ PEPROCESS Process,
	_Inout_opt_ PPS_CREATE_NOTIFY_INFO CreateInfo
	);

VOID HsMonitorProcess(
	_In_ PEPROCESS Process,
	_In_ HS_PROCESS_TYPE Type
	);

VOID HsUnMonitorProcess(
	_In_ PEPROCESS Process
	);

HS_PROCESS_TYPE HsIsProcessMonitored(
	_In_ PEPROCESS Process
	);

#endif