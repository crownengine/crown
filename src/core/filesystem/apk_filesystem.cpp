/*
 * Copyright (c) 2012-2016 Daniele Bartolini and individual contributors.
 * License: https://github.com/taylor001/crown/blob/master/LICENSE
 */

#include "config.h"

#if CROWN_PLATFORM_ANDROID

#include "apk_file.h"
#include "apk_filesystem.h"
#include "os.h"
#include "temp_allocator.h"
#include "vector.h"

namespace crown
{
ApkFilesystem::ApkFilesystem(Allocator& a, AAssetManager* asset_manager)
	: _allocator(&a)
	, _asset_manager(asset_manager)
{
}

File* ApkFilesystem::open(const char* path, FileOpenMode::Enum mode)
{
	CE_ASSERT_NOT_NULL(path);
	CE_ASSERT(mode == FileOpenMode::READ, "Cannot open for writing in Android assets folder");
	ApkFile* file = CE_NEW(*_allocator, ApkFile)(_asset_manager);
	file->open(path, mode);
	return file;
}

void ApkFilesystem::close(File& file)
{
	CE_DELETE(*_allocator, &file);
}

bool ApkFilesystem::exists(const char* path)
{
	return false;
}

bool ApkFilesystem::is_directory(const char* path)
{
	return true;
}

bool ApkFilesystem::is_file(const char* path)
{
	return true;
}

u64 ApkFilesystem::last_modified_time(const char* path)
{
	return 0;
}

void ApkFilesystem::create_directory(const char* /*path*/)
{
	CE_ASSERT(false, "Attempt to create directory in Android assets folder");
}

void ApkFilesystem::delete_directory(const char* /*path*/)
{
	CE_ASSERT(false, "Attempt to delete directory in Android assets folder");
}

void ApkFilesystem::create_file(const char* /*path*/)
{
	CE_ASSERT(false, "Attempt to create file in Android assets folder");
}

void ApkFilesystem::delete_file(const char* /*path*/)
{
	CE_ASSERT(false, "Attempt to delete file in Android assets folder");
}

void ApkFilesystem::list_files(const char* path, Vector<DynamicString>& files)
{
	CE_ASSERT_NOT_NULL(path);

	AAssetDir* root_dir = AAssetManager_openDir(_asset_manager, path);
	CE_ASSERT(root_dir != NULL, "Failed to open Android assets folder");

	const char* filename = NULL;
	while ((filename = AAssetDir_getNextFileName(root_dir)) != NULL)
	{
		TempAllocator512 ta;
		DynamicString name(filename, ta);
		vector::push_back(files, name);
	}

	AAssetDir_close(root_dir);
}

void ApkFilesystem::get_absolute_path(const char* path, DynamicString& os_path)
{
	os_path = path;
}

} // namespace crown

#endif // CROWN_PLATFORM_ANDROID
