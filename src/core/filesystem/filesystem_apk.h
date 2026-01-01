/*
 * Copyright (c) 2012-2026 Daniele Bartolini et al.
 * SPDX-License-Identifier: MIT
 */

#pragma once

#include "core/platform.h"

#if CROWN_PLATFORM_ANDROID
#include "core/filesystem/filesystem.h"
#include <sys/types.h> // off_t
#include <android/asset_manager.h>

namespace crown
{
/// Access files on Android's assets folder.
/// The assets folder is read-only and all the paths are relative.
///
/// @ingroup Filesystem
struct FilesystemApk : public Filesystem
{
	Allocator *_allocator;
	AAssetManager *_asset_manager;

	FilesystemApk(Allocator &a, AAssetManager *asset_manager);

	/// @copydoc Filesystem::open()
	File *open(const char *path, FileOpenMode::Enum mode) override;

	/// @copydoc Filesystem::close()
	void close(File &file) override;

	/// @copydoc Filesystem::stat()
	Stat stat(const char *path) override;

	/// @copydoc Filesystem::exists()
	bool exists(const char *path) override;

	/// @copydoc Filesystem::is_directory()
	bool is_directory(const char *path) override;

	/// @copydoc Filesystem::is_file()
	bool is_file(const char *path) override;

	/// @copydoc Filesystem::last_modified_time()
	u64 last_modified_time(const char *path) override;

	/// @copydoc Filesystem::create_directory()
	CreateResult create_directory(const char *path) override;

	/// @copydoc Filesystem::delete_directory()
	DeleteResult delete_directory(const char *path) override;

	/// @copydoc Filesystem::delete_file()
	DeleteResult delete_file(const char *path) override;

	/// @copydoc Filesystem::rename()
	RenameResult rename(const char *old_path, const char *new_path) override;

	/// @copydoc Filesystem::list_files()
	void list_files(const char *path, Vector<DynamicString> &files) override;

	/// @copydoc Filesystem::absolute_path()
	void absolute_path(DynamicString &os_path, const char *path) override;
};

} // namespace crown

#endif // if CROWN_PLATFORM_ANDROID
