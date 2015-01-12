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

using System;
using System.Collections.Generic;
using System.Linq;
using System.Runtime.InteropServices;
using System.Text;
using System.Threading.Tasks;

namespace HazardShield.Scanning.ClamAV
{
    public class ClamAV : IDisposable
    {
        const string CLAMAV_DLL = "libclamav.dll";
        const uint CL_INIT_DEFAULT = 0x0;

        [DllImport(CLAMAV_DLL, CallingConvention = CallingConvention.Cdecl)]
        static extern cl_error_t cl_init(uint initoptions);

        [DllImport(CLAMAV_DLL, CallingConvention = CallingConvention.Cdecl)]
        static extern IntPtr cl_retver();


        // Engine functions
        [DllImport(CLAMAV_DLL, CallingConvention = CallingConvention.Cdecl)]
        static extern IntPtr cl_engine_new();

        [DllImport(CLAMAV_DLL, CallingConvention = CallingConvention.Cdecl)]
        static extern cl_error_t cl_engine_set_num(IntPtr engine, cl_engine_field field, Int64 num);

        [DllImport(CLAMAV_DLL, CallingConvention = CallingConvention.Cdecl)]
        static extern Int64 cl_engine_get_num(IntPtr engine, cl_engine_field field, ref cl_error_t err);

        [DllImport(CLAMAV_DLL, CallingConvention = CallingConvention.Cdecl)]
        static extern cl_error_t cl_engine_set_str(IntPtr engine, cl_engine_field field, string str);

        [DllImport(CLAMAV_DLL, CallingConvention = CallingConvention.Cdecl)]
        static extern string cl_engine_get_str(IntPtr engine, cl_engine_field field, ref cl_error_t err);

        [DllImport(CLAMAV_DLL, CallingConvention = CallingConvention.Cdecl)]
        static extern cl_error_t cl_engine_compile(IntPtr engine);

        [DllImport(CLAMAV_DLL, CallingConvention = CallingConvention.Cdecl)]
        static extern cl_error_t cl_engine_free(IntPtr engine);


        // Scanning functions
        [DllImport(CLAMAV_DLL, CallingConvention = CallingConvention.Cdecl)]
        static extern cl_error_t cl_scanfile(string filename, out IntPtr virname, out uint scanned, IntPtr engine, scan_options scanoptions);


        // Database functions
        [DllImport(CLAMAV_DLL, CallingConvention = CallingConvention.Cdecl)]
        static extern cl_error_t cl_load(string path, IntPtr engine, ref uint signo, db_options dboptions);

        [DllImport(CLAMAV_DLL, CallingConvention = CallingConvention.Cdecl)]
        static extern IntPtr cl_cvdhead(string file);

        [DllImport(CLAMAV_DLL, CallingConvention = CallingConvention.Cdecl)]
        static extern void cl_cvdfree(IntPtr cvd);


        // Custom data scanning
        [DllImport(CLAMAV_DLL, CallingConvention = CallingConvention.Cdecl)]
        static extern IntPtr cl_fmap_open_memory(byte[] start, uint len);

        [DllImport(CLAMAV_DLL, CallingConvention = CallingConvention.Cdecl)]
        static extern void cl_fmap_close(IntPtr fmap);

        [DllImport(CLAMAV_DLL, CallingConvention = CallingConvention.Cdecl)]
        static extern cl_error_t cl_scanmap_callback(IntPtr map, out IntPtr virname, out uint scanned, IntPtr engine, scan_options scanoptions, IntPtr context);

        IntPtr Engine;

        public uint SignatureCount { get; private set; }

        public static string Version { get { return Marshal.PtrToStringAnsi(cl_retver()); } }

        static ClamAV()
        {
            cl_error_t result = cl_init(CL_INIT_DEFAULT);

            if (result != cl_error_t.CL_SUCCESS)
                throw new Exception(string.Format("cl_init failed with {0}", result));
        }

        public ClamAV()
        {
            Engine = cl_engine_new();
            SignatureCount = 0;

            if (Engine == IntPtr.Zero)
                throw new Exception("cl_engine_new() failed");
        }

        public bool LoadDatabase(string directory, db_options dbOptions)
        {
            uint signatureCount = 0;
            cl_error_t result = cl_load(directory, Engine, ref signatureCount, dbOptions);

            if (result == cl_error_t.CL_SUCCESS)
                SignatureCount += signatureCount;
            else
                Logger.Log(LogType.Error, "cl_load failed {0}", result);

            return result == cl_error_t.CL_SUCCESS;
        }

        public bool CompileDatabase()
        {
            cl_error_t result = cl_engine_compile(Engine);

            if (result != cl_error_t.CL_SUCCESS)
                Logger.Log(LogType.Error, "cl_engine_compile failed {0}", result);

            return result == cl_error_t.CL_SUCCESS;
        }

        public cl_error_t ScanFileEx(string filePath, scan_options scanoptions, out string threatName)
        {
            cl_error_t result;
            IntPtr virusName;
            uint scanned;

            result = cl_scanfile(filePath, out virusName, out scanned, Engine, scanoptions);

            if (result == cl_error_t.CL_VIRUS)
                threatName = Marshal.PtrToStringAnsi(virusName);
            else
                threatName = null;

            return result;
        }

        public cl_error_t ScanBuffer(byte[] buffer, uint bufferSize, scan_options scanoptions, out string threatName)
        {
            cl_error_t result;

            IntPtr fmap = cl_fmap_open_memory(buffer, bufferSize);

            if (fmap != IntPtr.Zero)
            {
                IntPtr virusName;
                uint scanned;

                result = cl_scanmap_callback(fmap, out virusName, out scanned, Engine, scanoptions, IntPtr.Zero);

                if (result == cl_error_t.CL_VIRUS)
                    threatName = Marshal.PtrToStringAnsi(virusName);
                else
                    threatName = null;

                cl_fmap_close(fmap);
            }
            else
            {
                threatName = null;
                result = cl_error_t.CL_EMEM;
            }

            return result;
        }

        public void Dispose()
        {
            if (Engine != IntPtr.Zero)
                cl_engine_free(Engine);
        }

        public static ClamDatabaseHeader GetDatabaseHeader(string filePath)
        {
            ClamDatabaseHeader database;
            IntPtr cvd = cl_cvdhead(filePath);

            if (cvd != IntPtr.Zero)
            {
                database = (ClamDatabaseHeader)Marshal.PtrToStructure(cvd, typeof(ClamDatabaseHeader));
                cl_cvdfree(cvd);
            }
            else
                database = default(ClamDatabaseHeader);

            return database;
        }

    }
}