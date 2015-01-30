#pragma once

#include "ClamDefs.h"
#include "FileToken.h"
#include <stdint.h>
#include <string>

namespace hzr
{
	class Scanner
	{
	public:
		__declspec(dllexport) Scanner();
		__declspec(dllexport) ~Scanner();

		__declspec(dllexport) ClamResult LoadClamDatabase(const std::string& directory, ClamDbOptions dbOptions);
		__declspec(dllexport) ClamResult CompileClamDatabase();
		__declspec(dllexport) ClamResult ScanBuffer(void* buffer, size_t length, ClamScanOptions scanOptions, const char** virusName);

		__declspec(dllexport) void Test();

		void FileCallback(const FileToken& fileToken) const;

		__declspec(dllexport) static ClamResult InitClamAV();

	private:
		void* ClamEngine;
		uint32_t Signatures;
	};
}