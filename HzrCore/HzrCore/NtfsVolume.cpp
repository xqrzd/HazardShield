#include "NtfsVolume.h"

#include <stdio.h>
#include <string>

BOOLEAN ReadSector(
	_In_ struct _NTFS_VOLUME* NtfsVolume,
	_In_ ULONGLONG Sector,
	_In_ ULONG SectorCount,
	_Out_ PVOID Buffer)
{
	BOOL result;
	HANDLE volumeHandle = NtfsVolume->Context;
	LARGE_INTEGER byteOffset;
	DWORD bytesToRead;
	DWORD bytesRead;

	bytesToRead = SectorCount * NtfsVolume->BytesPerSector;
	byteOffset.QuadPart = Sector * NtfsVolume->BytesPerSector;

	SetFilePointerEx(volumeHandle, byteOffset, NULL, FILE_BEGIN);

	result = ReadFile(volumeHandle, Buffer, bytesToRead, &bytesRead, NULL);

	return result;
}

//VOID StreamRead(
//	_In_ struct _NTFS_VOLUME* NtfsVolume,
//	_In_ PNTFS_ATTRIBUTE Attribute,
//	_In_ PVOID Buffer,
//	_In_ ULONG BufferSize,
//	_In_ PVOID Context)
//{
//	const char* virName;
//	NtfsFileToken* tk = (NtfsFileToken*)Context;
//	hzr::ClamResult res = scanner.ScanBuffer(Buffer, BufferSize, hzr::ClamScanOptions::STDOPT, &virName);
//
//	if (res == hzr::ClamResult::CL_VIRUS)
//	{
//		MessageBoxA(NULL, virName, NULL, NULL);
//	}
//	/*printf("Data: %s\n", Buffer);
//
//	if (Attribute->NameOffset && Attribute->NameLength)
//	{
//		WCHAR streamName[MAX_PATH];
//
//		RtlCopyMemory(streamName, NtfsOffsetToPointer(Attribute, Attribute->NameOffset), Attribute->NameLength * sizeof(WCHAR));
//		streamName[Attribute->NameLength] = L'\0';
//
//		wprintf(L"Name: %s\n", streamName);
//	}*/
//}

void NtfsVolume::MyCallback(const FileToken& fileToken)
{
	//MessageBoxW(NULL, fileToken.GetName().c_str(), fileToken.GetFullName().c_str(), 0);
	wprintf(L"%s\n", fileToken.GetFullName().c_str());
}

NtfsVolume::NtfsVolume(const std::wstring& volumeName)
	: VolumeName(volumeName)
{
	/*NtfsFileToken f(this, L"ksajfd", L"FULL", 0, true);
	f.EnumerateFileSystemInfos(std::bind(&NtfsVolume::MyCallback, this, std::placeholders::_1));*/
	/*hzr::Scanner::InitClamAV();
	scanner.LoadClamDatabase("C:\\database", hzr::ClamDbOptions::BYTECODE);
	scanner.CompileClamDatabase();*/

	Handle = CreateFileW(L"\\\\.\\C:", GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (Handle != INVALID_HANDLE_VALUE)
	{
		//NTFS_VOLUME ntfsVolume;
		NtfsInitVolume(ReadSector, 512, Handle, &Volume);

		/*FileToken::EnumFileCallback cl = std::bind(&NtfsVolume::MyCallback, this, std::placeholders::_1);
		IndexCallbackContext context(this, cl, L"C:");*/
		//context.callback = ;
		//context.Directory = L"C:";

		//NtfsEnumSubFiles(&ntfsVolume, 5, IndexCallback, &context); //241568
		//CloseHandle(volumeHandle);
		//NtfsFreeVolume(&ntfsVolume);

		//NtfsFileToken(this, std::wstring(), std::wstring(), 43, false);
	}
	else
		printf("CreateFileW failed %u\n", GetLastError());
}

NtfsVolume::~NtfsVolume()
{
	NtfsFreeVolume(&Volume);
	CloseHandle(Handle);
}

NtfsFileToken NtfsVolume::GetRootDirectory() const
{
	return NtfsFileToken(this, L"", VolumeName, MFT_RECORD_ROOT, 0, true);
}

bool NtfsVolume::EnumerateFileSystemInfos(const NtfsFileToken& fileToken, const FileToken::EnumFileCallback& callback) const
{
	IndexCallbackContext context(this, callback, fileToken.GetFullName());

	return NtfsEnumSubFiles(
		const_cast<PNTFS_VOLUME>(&Volume),
		fileToken.GetRecordNumber(),
		IndexCallback,
		&context) != 0;
}

bool NtfsVolume::ReadDataStreams(const NtfsFileToken& fileToken, const FileToken::FileStreamCallback& callback) const
{
	return false;
}

void NtfsVolume::IndexCallback(PNTFS_VOLUME NtfsVolume, PNTFS_INDEX_ENTRY IndexEntry, PVOID Context)
{
	if (IndexEntry->FileNameOffset &&
		IndexEntry->FileName.NameLength &&
		IndexEntry->FileName.NameSpace != ATTR_FILENAME_NAMESPACE_DOS &&
		IndexEntry->FileReference.RecordNumber > MFT_RECORD_USER &&
		!(IndexEntry->FileName.Flags & ATTR_FILENAME_FLAG_SPARSE))
	{
		// Make a copy of the context.
		IndexCallbackContext context(reinterpret_cast<IndexCallbackContext*>(Context));

		context.Directory.append(L"\\");
		context.Directory.append(IndexEntry->FileName.Name, IndexEntry->FileName.NameLength);

		NtfsFileToken fileToken = NtfsFileToken(
			context.Instance,
			std::wstring(IndexEntry->FileName.Name,
			IndexEntry->FileName.NameLength),
			context.Directory,
			IndexEntry->FileReference.RecordNumber,
			IndexEntry->FileName.RealSize,
			(IndexEntry->FileName.Flags & ATTR_FILENAME_FLAG_DIRECTORY) > 0);

		wprintf(L"%u %s\n", IndexEntry->FileReference.RecordNumber, context.Directory.c_str());

		context.Callback(fileToken);

		/*if (IndexEntry->FileName.Flags & ATTR_FILENAME_FLAG_DIRECTORY)
		{
			NtfsEnumSubFiles(NtfsVolume, IndexEntry->FileReference.RecordNumber, IndexCallback, &newr);
		}
		else
		{
			NtfsReadFileDataStreams(NtfsVolume, IndexEntry->FileReference.RecordNumber, StreamRead, &tk);
		}*/
	}
}

void NtfsVolume::StreamCallback(PNTFS_VOLUME NtfsVolume, PNTFS_ATTRIBUTE Attribute, PVOID Buffer, ULONG BufferSize, PVOID Context)
{

}