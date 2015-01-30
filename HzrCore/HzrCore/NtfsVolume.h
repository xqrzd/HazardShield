#pragma once

#include <functional>
#include "Ntfs.h"
#include "NtfsFileToken.h"

class NtfsFileToken;

class NtfsVolume
{
public:
	NtfsVolume(const std::wstring& volumeName);
	~NtfsVolume();

	NtfsFileToken GetRootDirectory() const;
	bool EnumerateFileSystemInfos(const NtfsFileToken& fileToken, const FileToken::EnumFileCallback& callback) const;
	bool ReadDataStreams(const NtfsFileToken& fileToken, const FileToken::FileStreamCallback& callback) const;

private:
	struct IndexCallbackContext
	{
		IndexCallbackContext::IndexCallbackContext(
			const NtfsVolume* instance,
			const FileToken::EnumFileCallback& callback,
			const std::wstring& directory)
			: Instance(instance),
			Callback(callback),
			Directory(directory)
		{
		}

		IndexCallbackContext::IndexCallbackContext(
			const IndexCallbackContext* context)
			: Instance(context->Instance),
			Callback(context->Callback),
			Directory(context->Directory)
		{
		}

		const NtfsVolume* Instance;
		const FileToken::EnumFileCallback& Callback;
		std::wstring Directory;
	};

	struct StreamCallbackContext
	{
	};

	NTFS_VOLUME Volume;
	const std::wstring VolumeName;
	HANDLE Handle;

	void MyCallback(const FileToken& fileToken);

	static void IndexCallback(PNTFS_VOLUME NtfsVolume, PNTFS_INDEX_ENTRY IndexEntry, PVOID Context);
	static void StreamCallback(PNTFS_VOLUME NtfsVolume, PNTFS_ATTRIBUTE Attribute, PVOID Buffer, ULONG BufferSize, PVOID Context);
};