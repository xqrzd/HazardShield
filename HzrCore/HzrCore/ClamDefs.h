#pragma once

#include <stdint.h>

namespace hzr
{
	enum class ClamResult : int32_t
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
	};

	enum class ClamDbOptions : uint32_t
	{
		PHISHING = 0x2,
		PHISHING_URLS = 0x8,
		PUA = 0x10,
		CVDNOTMP = 0x20,    /* obsolete */
		OFFICIAL = 0x40,    /* internal */
		PUA_MODE = 0x80,
		PUA_INCLUDE = 0x100,
		PUA_EXCLUDE = 0x200,
		COMPILED = 0x400,   /* internal */
		DIRECTORY = 0x800,   /* internal */
		OFFICIAL_ONLY = 0x1000,
		BYTECODE = 0x2000,
		SIGNED = 0x4000,  /* internal */
		BYTECODE_UNSIGNED = 0x8000,
		UNSIGNED = 0x10000, /* internal */
		BYTECODE_STATS = 0x20000,
		ENHANCED = 0x40000,
		STDOPT = (PHISHING | PHISHING_URLS | BYTECODE)
	};

	inline ClamDbOptions operator | (ClamDbOptions lhs, ClamDbOptions rhs)
	{
		return static_cast<ClamDbOptions>(static_cast<uint32_t>(lhs) | static_cast<uint32_t>(rhs));
	}

	enum class ClamScanOptions : uint32_t
	{
		RAW = 0x0,
		ARCHIVE = 0x1,
		MAIL = 0x2,
		OLE2 = 0x4,
		BLOCKENCRYPTED = 0x8,
		HTML = 0x10,
		PE = 0x20,
		BLOCKBROKEN = 0x40,
		MAILURL = 0x80, /* ignored */
		BLOCKMAX = 0x100, /* ignored */
		ALGORITHMIC = 0x200,
		PHISHING_BLOCKSSL = 0x800, /* ssl mismatches, not ssl by itself*/
		PHISHING_BLOCKCLOAK = 0x1000,
		ELF = 0x2000,
		PDF = 0x4000,
		STRUCTURED = 0x8000,
		STRUCTURED_SSN_NORMAL = 0x10000,
		STRUCTURED_SSN_STRIPPED = 0x20000,
		PARTIAL_MESSAGE = 0x40000,
		HEURISTIC_PRECEDENCE = 0x80000,
		BLOCKMACROS = 0x100000,
		ALLMATCHES = 0x200000,
		SWF = 0x400000,
		PARTITION_INTXN = 0x800000,
		FILE_PROPERTIES = 0x10000000,
		//UNUSED=0x20000000
		PERFORMANCE_INFO = 0x40000000, /* collect performance timings */
		INTERNAL_COLLECT_SHA = 0x80000000, /* Enables hash output in sha-collect builds - for internal use only */

		/* recommended scan settings */
		STDOPT = (ARCHIVE | MAIL | OLE2 | PDF | HTML | PE | ALGORITHMIC | ELF | SWF)
	};

	inline ClamScanOptions operator | (ClamScanOptions lhs, ClamScanOptions rhs)
	{
		return static_cast<ClamScanOptions>(static_cast<uint32_t>(lhs) | static_cast<uint32_t>(rhs));
	}
}