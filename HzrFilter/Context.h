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

#ifndef HZRFILTER_CONTEXT_H
#define HZRFILTER_CONTEXT_H

#include <fltKernel.h>

extern const FLT_CONTEXT_REGISTRATION ContextRegistration[];

typedef struct _FILTER_INSTANCE_CONTEXT {
	BOOLEAN CacheSupported;
} FILTER_INSTANCE_CONTEXT, *PFILTER_INSTANCE_CONTEXT;

typedef struct _FILTER_STREAMHANDLE_CONTEXT {
	BOOLEAN PrefetchOpen;
} FILTER_STREAMHANDLE_CONTEXT, *PFILTER_STREAMHANDLE_CONTEXT;

#endif