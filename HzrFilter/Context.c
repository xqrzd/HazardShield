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

#include "Context.h"

VOID HzrFilterInstanceContextCleanup(
	_In_ PFLT_CONTEXT Context,
	_In_ FLT_CONTEXT_TYPE ContextType
	);

const FLT_CONTEXT_REGISTRATION ContextRegistration[] = {
	{
		FLT_INSTANCE_CONTEXT,
		0,
		HzrFilterInstanceContextCleanup,
		sizeof(FILTER_INSTANCE_CONTEXT),
		'xtci'
	},
	{
		FLT_STREAMHANDLE_CONTEXT,
		0,
		NULL,
		sizeof(FILTER_STREAMHANDLE_CONTEXT),
		'hctx'
	},

	{ FLT_CONTEXT_END }
};

VOID HzrFilterInstanceContextCleanup(
	_In_ PFLT_CONTEXT Context,
	_In_ FLT_CONTEXT_TYPE ContextType)
{
	PFILTER_INSTANCE_CONTEXT context = (PFILTER_INSTANCE_CONTEXT)Context;

	UNREFERENCED_PARAMETER(ContextType);

	if (context->CacheSupported)
	{
		// Cleanup cache, etc.
	}
}