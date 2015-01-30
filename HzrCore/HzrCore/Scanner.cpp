#include "clamav.h"
#include "Scanner.h"
#include "NtfsVolume.h"

namespace hzr
{
	Scanner::Scanner()
	{
		ClamEngine = cl_engine_new();
		Signatures = 0;

		//NtfsVolume v;
	}

	Scanner::~Scanner()
	{
		if (ClamEngine)
			cl_engine_free(reinterpret_cast<cl_engine*>(ClamEngine));
	}

	ClamResult Scanner::LoadClamDatabase(const std::string& directory, ClamDbOptions dbOptions)
	{
		return static_cast<ClamResult>(cl_load(
			directory.c_str(),
			reinterpret_cast<cl_engine*>(ClamEngine),
			&Signatures,
			static_cast<unsigned int>(dbOptions)));
	}

	ClamResult Scanner::CompileClamDatabase()
	{
		return static_cast<ClamResult>(cl_engine_compile(reinterpret_cast<cl_engine*>(ClamEngine)));
	}

	ClamResult Scanner::ScanBuffer(void* buffer, size_t length, ClamScanOptions scanOptions, const char** virusName)
	{
		ClamResult result;
		cl_fmap* map = cl_fmap_open_memory(buffer, length);

		if (map)
		{
			result = static_cast<ClamResult>(cl_scanmap_callback(map, virusName, NULL, reinterpret_cast<cl_engine*>(ClamEngine), static_cast<unsigned int>(scanOptions), NULL));

			cl_fmap_close(map);
		}
		else
			result = ClamResult::CL_EMAP;

		return result;
	}

	void Scanner::Test()
	{
		NtfsVolume v(L"C:");
		//FileToken& fi = v.GetRootDirectory();

		//const NtfsVolume* volume, const std::wstring& name, const std::wstring& fullName, uint32_t recordNumber, uint64_t fileSize, bool isDirectory
		NtfsFileToken fi(&v, L"AppData", L"C:\\Users\\Andy\\AppData", 32947, 0, true);


		FileToken::EnumFileCallback callback = std::bind(&Scanner::FileCallback, this, std::placeholders::_1);

		fi.EnumerateFileSystemInfos(callback);
	}

	//uint64_t counter = 0;

	void Scanner::FileCallback(const FileToken& fileToken) const
	{
		/*counter++;
		wprintf(L"File: %s\n", fileToken.GetFullName().c_str());*/

		//MessageBox(NULL, fileToken.GetFullName().c_str(), fileToken.GetName().c_str(), 0);
		if (fileToken.IsDirectory())
		{
			FileToken::EnumFileCallback callback = std::bind(&Scanner::FileCallback, this, std::placeholders::_1);
			fileToken.EnumerateFileSystemInfos(callback);
		}
	}

	ClamResult Scanner::InitClamAV()
	{
		return static_cast<ClamResult>(cl_init(CL_INIT_DEFAULT));
	}
}