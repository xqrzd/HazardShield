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

#include <Windows.h>

LRESULT CALLBACK GetMsgProc(
	_In_  int code,
	_In_  WPARAM wParam,
	_In_  LPARAM lParam)
{
	if (code == HC_ACTION)
	{
		PMSG msg = (PMSG)lParam;

		if (msg->message == WM_CLOSE ||
			msg->message == WM_DESTROY ||
			msg->message == WM_QUIT ||
			msg->message == WM_NCDESTROY)
		{
			msg->message = 0;
		}
		else if (msg->message == WM_SYSCOMMAND && msg->wParam == SC_CLOSE)
		{
			msg->message = 0;
		}
	}

	return CallNextHookEx(NULL, code, wParam, lParam);
}