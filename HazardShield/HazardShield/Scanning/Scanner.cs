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

using HazardShield.Core;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Runtime.InteropServices;
using System.Text;
using System.Threading.Tasks;

namespace HazardShield.Scanning
{
    public class Scanner : IDisposable
    {
        [DllImport("libclamav.dll", CallingConvention = CallingConvention.Cdecl)]
        static extern IntPtr cl_retver();

        public static string ClamAVVersion { get { return Marshal.PtrToStringAnsi(cl_retver()); } }

        IntPtr HsScanner;

        MemoryObjectCallback MemoryObjectCallbackPtr;

        public Scanner()
        {
            MemoryObjectCallbackPtr = MemoryObjectScanned;

            HzrCore.CreateScanner(out HsScanner, MemoryObjectCallbackPtr);
        }

        public bool ScanMemory(IntPtr processId)
        {
            return HzrCore.ScanMemory(HsScanner, processId);
        }

        void MemoryObjectScanned(HS_MEMORY_OBJECT mem, string vir)
        {
        }

        public void Dispose()
        {
            if (HsScanner != IntPtr.Zero)
                HzrCore.DeleteScanner(HsScanner);
        }
    }
}