using System;
using System.Collections.Generic;
using System.Linq;
using System.Runtime.InteropServices;
using System.Text;
using System.Threading.Tasks;

namespace HazardShield.Core
{
    [StructLayout(LayoutKind.Sequential)]
    public struct HS_MEMORY_OBJECT
    {
        public IntPtr BaseAddress;
        public IntPtr Size;
        public IntPtr ProcessId;

        public uint Protection;
        public uint Type;
    }
}