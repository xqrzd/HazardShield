#pragma once

#include "FileToken.h"
#include "NtfsVolume.h"

class NtfsVolume;

class NtfsFileToken : public FileToken
{
public:
	NtfsFileToken(const NtfsVolume* volume, const std::wstring& name, const std::wstring& fullName, uint32_t recordNumber, uint64_t fileSize, bool isDirectory);

	inline const std::wstring& GetName() const { return Name; };
	inline const std::wstring& GetFullName() const { return FullName; };
	inline bool IsDirectory() const { return Directory; };
	inline uint32_t GetRecordNumber() const { return RecordNumber; };

	bool EnumerateFileSystemInfos(const EnumFileCallback& callback) const;
	bool ReadDataStreams(const FileStreamCallback& callback) const;

private:
	const std::wstring Name;
	const std::wstring FullName;

	const uint32_t RecordNumber;
	const uint64_t FileSize;
	const bool Directory;

	const NtfsVolume* Volume;
};