/*
 * Copyright (c) 2012-2021 Daniele Bartolini et al.
 * License: https://github.com/dbartolini/crown/blob/master/LICENSE
 */

#include "core/platform.h"

#if CROWN_PLATFORM_ANDROID

#include "core/containers/vector.inl"
#include "core/filesystem/file.h"
#include "core/filesystem/filesystem_apk.h"
#include "core/memory/temp_allocator.inl"
#include "core/os.h"
#include "core/strings/dynamic_string.inl"

namespace crown
{
struct FileApk : public File
{
	AAssetManager* _asset_manager;
	AAsset* _asset;

	explicit FileApk(AAssetManager* asset_manager)
		: _asset_manager(asset_manager)
		, _asset(NULL)
	{
	}

	virtual ~FileApk()
	{
		close();
	}

	void open(const char* path, FileOpenMode::Enum /*mode*/)
	{
		_asset = AAssetManager_open(_asset_manager, path, AASSET_MODE_RANDOM);
	}

	void close()
	{
		if (_asset)
		{
			AAsset_close(_asset);
			_asset = NULL;
		}
	}

	bool is_open()
	{
		return _asset != NULL;
	}

	u32 size()
	{
		CE_ASSERT(is_open(), "File is not open");
		return AAsset_getLength(_asset);
	}

	u32 position()
	{
		CE_ASSERT(is_open(), "File is not open");
		return u32(AAsset_getLength(_asset) - AAsset_getRemainingLength(_asset));
	}

	bool end_of_file()
	{
		CE_ASSERT(is_open(), "File is not open");
		return AAsset_getRemainingLength(_asset) == 0;
	}

	void seek(u32 position)
	{
		CE_ASSERT(is_open(), "File is not open");
		off_t seek_result = AAsset_seek(_asset, (off_t)position, SEEK_SET);
		CE_ASSERT(seek_result != (off_t)-1, "AAsset_seek: error");
		CE_UNUSED(seek_result);
	}

	void seek_to_end()
	{
		CE_ASSERT(is_open(), "File is not open");
		off_t seek_result = AAsset_seek(_asset, 0, SEEK_END);
		CE_ASSERT(seek_result != (off_t)-1, "AAsset_seek: error");
		CE_UNUSED(seek_result);
	}

	void skip(u32 bytes)
	{
		CE_ASSERT(is_open(), "File is not open");
		off_t seek_result = AAsset_seek(_asset, (off_t)bytes, SEEK_CUR);
		CE_ASSERT(seek_result != (off_t)-1, "AAsset_seek: error");
		CE_UNUSED(seek_result);
	}

	u32 read(void* data, u32 size)
	{
		CE_ASSERT(is_open(), "File is not open");
		CE_ENSURE(NULL != data);
		return (u32)AAsset_read(_asset, data, size);
	}

	u32 write(const void* /*data*/, u32 /*size*/)
	{
		CE_ASSERT(is_open(), "File is not open");
		CE_FATAL("Apk files are read only!");
		return 0;
	}

	void flush()
	{
		// Not needed
	}
};

FilesystemApk::FilesystemApk(Allocator& a, AAssetManager* asset_manager)
	: _allocator(&a)
	, _asset_manager(asset_manager)
{
}

File* FilesystemApk::open(const char* path, FileOpenMode::Enum mode)
{
	CE_ENSURE(NULL != path);
	CE_ASSERT(mode == FileOpenMode::READ, "Cannot open for writing in Android assets folder");
	FileApk* file = CE_NEW(*_allocator, FileApk)(_asset_manager);
	file->open(path, mode);
	return file;
}

void FilesystemApk::close(File& file)
{
	CE_DELETE(*_allocator, &file);
}

Stat FilesystemApk::stat(const char* /*path*/)
{
	Stat info;
	info.file_type = Stat::REGULAR;
	info.size = 0;
	info.mtime = UINT64_MAX;
	return info;
}

bool FilesystemApk::exists(const char* /*path*/)
{
	return true;
}

bool FilesystemApk::is_directory(const char* path)
{
	return true;
}

bool FilesystemApk::is_file(const char* path)
{
	return true;
}

u64 FilesystemApk::last_modified_time(const char* path)
{
	return 0;
}

CreateResult FilesystemApk::create_directory(const char* /*path*/)
{
	CE_FATAL("Cannot create directory in Android assets folder");
	CreateResult cr;
	cr.error = CreateResult::UNKNOWN;
	return cr;
}

DeleteResult FilesystemApk::delete_directory(const char* /*path*/)
{
	CE_FATAL("Cannot delete directory in Android assets folder");
	DeleteResult dr;
	dr.error = DeleteResult::UNKNOWN;
	return dr;
}

DeleteResult FilesystemApk::delete_file(const char* /*path*/)
{
	CE_FATAL("Cannot delete file in Android assets folder");
	DeleteResult dr;
	dr.error = DeleteResult::UNKNOWN;
	return dr;
}

void FilesystemApk::list_files(const char* path, Vector<DynamicString>& files)
{
	CE_ENSURE(NULL != path);

	AAssetDir* root_dir = AAssetManager_openDir(_asset_manager, path);
	CE_ASSERT(root_dir != NULL, "Failed to open Android assets folder");

	const char* filename = NULL;
	while ((filename = AAssetDir_getNextFileName(root_dir)) != NULL)
	{
		TempAllocator512 ta;
		DynamicString name(ta);
		name = filename;
		vector::push_back(files, name);
	}

	AAssetDir_close(root_dir);
}

void FilesystemApk::absolute_path(DynamicString& os_path, const char* path)
{
	os_path = path;
}

} // namespace crown

#endif // CROWN_PLATFORM_ANDROID
