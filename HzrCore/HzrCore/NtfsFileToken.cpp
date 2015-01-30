#include "NtfsFileToken.h"

NtfsFileToken::NtfsFileToken(const NtfsVolume* volume, const std::wstring& name, const std::wstring& fullName, uint32_t recordNumber, uint64_t fileSize, bool isDirectory)
	: Volume(volume), Name(name), FullName(fullName), RecordNumber(recordNumber), FileSize(fileSize), Directory(isDirectory)
{
}

bool NtfsFileToken::EnumerateFileSystemInfos(const EnumFileCallback& callback) const
{
	return Volume->EnumerateFileSystemInfos(*this, callback);
}

bool NtfsFileToken::ReadDataStreams(const FileStreamCallback& callback) const
{
	return false;
}