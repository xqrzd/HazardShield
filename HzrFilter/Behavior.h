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

typedef enum _EXPL_PROCESS_TYPE {
	EXPL_PROCESS_NONE = 0,
	EXPL_PROCESS_JAVA,
	EXPL_PROCESS_FIREFOX_PLUGIN_HOST,
	EXPL_PROCESS_CHROME_OPERA_PLUGIN_HOST
} EXPL_PROCESS_TYPE, *PEXPL_PROCESS_TYPE;

typedef struct _EXPL_PROCESS {
	PEPROCESS Process;
	EXPL_PROCESS_TYPE Type;
} EXPL_PROCESS, *PEXPL_PROCESS;

VOID HzrExplInit(
	);

VOID HzrExplFree(
	);

BOOLEAN HzrExplPreWrite(
	_Inout_ PFLT_CALLBACK_DATA Data,
	_In_ PCFLT_RELATED_OBJECTS FltObjects
	);

VOID HzrExplCreateProcessNotifyEx(
	_Inout_ PEPROCESS Process,
	_Inout_opt_ PPS_CREATE_NOTIFY_INFO CreateInfo
	);

EXPL_PROCESS_TYPE HzrExplCheckNewProcess(
	_In_ PCUNICODE_STRING ImageFileName,
	_In_opt_ PCUNICODE_STRING CommandLine
	);

VOID HzrExplAddExploitProcess(
	_In_ PEPROCESS Process,
	_In_ EXPL_PROCESS_TYPE Type
	);

VOID HzrExplRemoveExploitProcess(
	_In_ PEPROCESS Process
	);

EXPL_PROCESS_TYPE HzrExplIsProcessExploit(
	_In_ PEPROCESS Process
	);

#endif