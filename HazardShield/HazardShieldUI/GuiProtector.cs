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

using HazardShield.Win32;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;

namespace HazardShieldUI
{
    class GuiProtector
    {
        IntPtr Hook;

        public GuiProtector()
        {
            Hook = IntPtr.Zero;
        }

        public bool StartProtection()
        {
            IntPtr module = WinApi.LoadLibrary("HzrWndHook.dll");

            if (module != IntPtr.Zero)
            {
                IntPtr func = WinApi.GetProcAddress(module, "GetMsgProc");
                if (func != IntPtr.Zero)
                {
                    Hook = WinApi.SetWindowsHookEx(3, func, module, WinApi.GetCurrentThreadId());

                    if (Hook == IntPtr.Zero)
                        MessageBox.Show("SetWindowsHookEx failed");
                }
                else
                    MessageBox.Show("GetProcAddress failed");
            }
            else
                MessageBox.Show("LoadLibrary failed");

            return Hook != IntPtr.Zero;
        }

        public bool DisableProtection()
        {
            return WinApi.UnhookWindowsHookEx(Hook);
        }
    }
}