#pragma once

#include <functional>
#include <stdint.h>
#include <string>

class FileToken
{
public:
	typedef std::function<void(const FileToken& fileToken)> EnumFileCallback;
	typedef std::function<void(const FileToken& fileToken, const std::wstring& streamName, const void* buffer, uint64_t bufferSize)> FileStreamCallback;

	virtual const std::wstring& GetName() const = 0;
	virtual const std::wstring& GetFullName() const = 0;
	virtual bool IsDirectory() const = 0;

	virtual bool EnumerateFileSystemInfos(const EnumFileCallback& callback) const = 0;
	virtual bool ReadDataStreams(const FileStreamCallback& callback) const = 0;
};