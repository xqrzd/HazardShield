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
using System.Text;
using System.Threading.Tasks;

namespace HazardShield.ClamAV
{
    public enum cl_error_t
    {
        /* libclamav specific */
        CL_CLEAN = 0,
        CL_SUCCESS = 0,
        CL_VIRUS,
        CL_ENULLARG,
        CL_EARG,
        CL_EMALFDB,
        CL_ECVD,
        CL_EVERIFY,
        CL_EUNPACK,

        /* I/O and memory errors */
        CL_EOPEN,
        CL_ECREAT,
        CL_EUNLINK,
        CL_ESTAT,
        CL_EREAD,
        CL_ESEEK,
        CL_EWRITE,
        CL_EDUP,
        CL_EACCES,
        CL_ETMPFILE,
        CL_ETMPDIR,
        CL_EMAP,
        CL_EMEM,
        CL_ETIMEOUT,

        /* internal (not reported outside libclamav) */
        CL_BREAK,
        CL_EMAXREC,
        CL_EMAXSIZE,
        CL_EMAXFILES,
        CL_EFORMAT,
        CL_EPARSE,
        CL_EBYTECODE,/* may be reported in testmode */
        CL_EBYTECODE_TESTFAIL, /* may be reported in testmode */

        /* c4w error codes */
        CL_ELOCK,
        CL_EBUSY,
        CL_ESTATE,

        /* no error codes below this line please */
        CL_ELAST_ERROR
    }

    [Flags]
    public enum db_options : uint
    {
        CL_DB_PHISHING = 0x2,
        CL_DB_PHISHING_URLS = 0x8,
        CL_DB_PUA = 0x10,
        CL_DB_CVDNOTMP = 0x20,    /* obsolete */
        CL_DB_OFFICIAL = 0x40,    /* internal */
        CL_DB_PUA_MODE = 0x80,
        CL_DB_PUA_INCLUDE = 0x100,
        CL_DB_PUA_EXCLUDE = 0x200,
        CL_DB_COMPILED = 0x400,   /* internal */
        CL_DB_DIRECTORY = 0x800,   /* internal */
        CL_DB_OFFICIAL_ONLY = 0x1000,
        CL_DB_BYTECODE = 0x2000,
        CL_DB_SIGNED = 0x4000,  /* internal */
        CL_DB_BYTECODE_UNSIGNED = 0x8000,
        CL_DB_UNSIGNED = 0x10000, /* internal */
        CL_DB_BYTECODE_STATS = 0x20000,
        CL_DB_ENHANCED = 0x40000
    }

    [Flags]
    public enum scan_options : uint
    {
        CL_SCAN_RAW = 0x0,
        CL_SCAN_ARCHIVE = 0x1,
        CL_SCAN_MAIL = 0x2,
        CL_SCAN_OLE2 = 0x4,
        CL_SCAN_BLOCKENCRYPTED = 0x8,
        CL_SCAN_HTML = 0x10,
        CL_SCAN_PE = 0x20,
        CL_SCAN_BLOCKBROKEN = 0x40,
        CL_SCAN_MAILURL = 0x80, /* ignored */
        CL_SCAN_BLOCKMAX = 0x100, /* ignored */
        CL_SCAN_ALGORITHMIC = 0x200,
        CL_SCAN_PHISHING_BLOCKSSL = 0x800, /* ssl mismatches, not ssl by itself*/
        CL_SCAN_PHISHING_BLOCKCLOAK = 0x1000,
        CL_SCAN_ELF = 0x2000,
        CL_SCAN_PDF = 0x4000,
        CL_SCAN_STRUCTURED = 0x8000,
        CL_SCAN_STRUCTURED_SSN_NORMAL = 0x10000,
        CL_SCAN_STRUCTURED_SSN_STRIPPED = 0x20000,
        CL_SCAN_PARTIAL_MESSAGE = 0x40000,
        CL_SCAN_HEURISTIC_PRECEDENCE = 0x80000,
        CL_SCAN_BLOCKMACROS = 0x100000,
        CL_SCAN_ALLMATCHES = 0x200000,
        CL_SCAN_SWF = 0x400000,
        CL_SCAN_PARTITION_INTXN = 0x800000,
        CL_SCAN_FILE_PROPERTIES = 0x10000000,
        // Unused 0x20000000
        CL_SCAN_PERFORMANCE_INFO = 0x40000000, /* collect performance timings */
        CL_SCAN_INTERNAL_COLLECT_SHA = 0x80000000, /* Enables hash output in sha-collect builds - for internal use only */
        CL_SCAN_STDOPT = (CL_SCAN_ARCHIVE | CL_SCAN_MAIL | CL_SCAN_OLE2 | CL_SCAN_PDF | CL_SCAN_HTML | CL_SCAN_PE | CL_SCAN_ALGORITHMIC | CL_SCAN_ELF | CL_SCAN_SWF)
    }

    public enum cl_engine_field
    {
        CL_ENGINE_MAX_SCANSIZE,	    /* uint64_t */
        CL_ENGINE_MAX_FILESIZE,	    /* uint64_t */
        CL_ENGINE_MAX_RECURSION,	    /* uint32_t	*/
        CL_ENGINE_MAX_FILES,	    /* uint32_t */
        CL_ENGINE_MIN_CC_COUNT,	    /* uint32_t */
        CL_ENGINE_MIN_SSN_COUNT,	    /* uint32_t */
        CL_ENGINE_PUA_CATEGORIES,	    /* (char *) */
        CL_ENGINE_DB_OPTIONS,	    /* uint32_t */
        CL_ENGINE_DB_VERSION,	    /* uint32_t */
        CL_ENGINE_DB_TIME,		    /* time_t */
        CL_ENGINE_AC_ONLY,		    /* uint32_t */
        CL_ENGINE_AC_MINDEPTH,	    /* uint32_t */
        CL_ENGINE_AC_MAXDEPTH,	    /* uint32_t */
        CL_ENGINE_TMPDIR,		    /* (char *) */
        CL_ENGINE_KEEPTMP,		    /* uint32_t */
        CL_ENGINE_BYTECODE_SECURITY,    /* uint32_t */
        CL_ENGINE_BYTECODE_TIMEOUT,     /* uint32_t */
        CL_ENGINE_BYTECODE_MODE,        /* uint32_t */
        CL_ENGINE_MAX_EMBEDDEDPE,       /* uint64_t */
        CL_ENGINE_MAX_HTMLNORMALIZE,    /* uint64_t */
        CL_ENGINE_MAX_HTMLNOTAGS,       /* uint64_t */
        CL_ENGINE_MAX_SCRIPTNORMALIZE,  /* uint64_t */
        CL_ENGINE_MAX_ZIPTYPERCG,       /* uint64_t */
        CL_ENGINE_FORCETODISK,          /* uint32_t */
        CL_ENGINE_DISABLE_CACHE,        /* uint32_t */
        CL_ENGINE_DISABLE_PE_STATS,     /* uint32_t */
        CL_ENGINE_STATS_TIMEOUT,        /* uint32_t */
        CL_ENGINE_MAX_PARTITIONS,       /* uint32_t */
        CL_ENGINE_MAX_ICONSPE,          /* uint32_t */
        CL_ENGINE_TIME_LIMIT            /* uint32_t */
    }

    public struct ClamDatabaseHeader
    {
        public string BuildTime;
        public uint Version;
        public uint Signatures;
        public uint FunctionalityLevel;
        public string MD5;
        public string DigitalSignature;
        public string Builder;
        public uint BuildTimeAsLong;
    };
}