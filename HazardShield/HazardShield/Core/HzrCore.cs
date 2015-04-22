using HazardShield.ClamAV;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Runtime.InteropServices;
using System.Text;
using System.Threading.Tasks;

namespace HazardShield.Core
{
    [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
    public delegate void MemoryObjectCallback(HS_MEMORY_OBJECT obj, string vir);

    public static class HzrCore
    {
        const string HzrCoreDll = "HzrCore.dll";
        const CallingConvention CallConvention = CallingConvention.Cdecl;

        [DllImport(HzrCoreDll, CallingConvention = CallConvention)]
        static extern cl_error_t HzrInitClamAv();

        [DllImport(HzrCoreDll, CallingConvention = CallConvention)]
        static extern bool HzrCreateScanner(out IntPtr Scanner, MemoryObjectCallback MemoryObjectCallback);

        [DllImport(HzrCoreDll, CallingConvention = CallConvention)]
        static extern void HzrDeleteScanner(IntPtr Scanner);

        [DllImport(HzrCoreDll, CallingConvention = CallConvention)]
        static extern bool HsScanProcessMemoryBasic(IntPtr Scanner, IntPtr ProcessId);

        static HzrCore()
        {
            cl_error_t result = HzrInitClamAv();

            if (result != cl_error_t.CL_SUCCESS)
                throw new Exception(string.Format("HzrInitClamAv failed {0}", result));
        }

        public static bool CreateScanner(out IntPtr scanner, MemoryObjectCallback memoryObjectCallback)
        {
            return HzrCreateScanner(out scanner, memoryObjectCallback);
        }

        public static void DeleteScanner(IntPtr scanner)
        {
            HzrDeleteScanner(scanner);
        }

        public static bool ScanMemory(IntPtr scanner, IntPtr processId)
        {
            return HsScanProcessMemoryBasic(scanner, processId);
        }

    }
}