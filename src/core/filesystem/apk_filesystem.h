/*
 * Copyright (c) 2012-2015 Daniele Bartolini and individual contributors.
 * License: https://github.com/taylor001/crown/blob/master/LICENSE
 */

#pragma once

#include "config.h"

#if CROWN_PLATFORM_ANDROID

#include "filesystem.h"
#include <sys/types.h> // off_t
#include <android/asset_manager.h>

namespace crown
{

/// Access files on Android's assets folder.
/// The assets folder is read-only and all the paths are relative.
class ApkFilesystem : public Filesystem
{
public:

	ApkFilesystem(AAssetManager* asset_manager);

	/// @copydoc Filesystem::open()
	/// @note
	/// @a mode can only be FOM_READ
	File* open(const char* rel_path, FileOpenMode mode);

	/// @copydoc Filesystem::close()
	void close(File* file);

	/// Returns always false under Android.
	bool exists(const char* path);

	/// Returns always false under Android.
	bool is_directory(const char* path);

	/// Returns always false under Android.
	bool is_file(const char* path);

	/// Stub method, assets folder is read-only.
	void create_directory(const char* path);

	/// Stub method, assets folder is read-only.
	void delete_directory(const char* path);

	/// Stub method, assets folder is read-only.
	void create_file(const char* path);

	/// Stub method, assets folder is read-only.
	void delete_file(const char* path);

	/// @copydoc Filesystem::list_files().
	void list_files(const char* path, Vector<DynamicString>& files);

	/// Returns the absolute path of the given @a path.
	/// @note
	/// Assets folder has no concept of "absolute path", all paths are
	/// relative to the assets folder itself, so, all paths are returned unchanged.
	void get_absolute_path(const char* path, DynamicString& os_path);

private:

	AAssetManager* _asset_manager;
};

} // namespace crown

#endif // CROWN_PLATFORM_ANDROID
