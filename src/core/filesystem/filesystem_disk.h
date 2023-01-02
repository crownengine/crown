/*
 * Copyright (c) 2012-2023 Daniele Bartolini et al.
 * SPDX-License-Identifier: MIT
 */

#pragma once

#include "core/filesystem/filesystem.h"
#include "core/strings/dynamic_string.h"

namespace crown
{
/// Access files on disk.
/// All the file paths can be either relative or absolute.
/// When a relative path is given, it is automatically translated
/// to its absolute counterpart based on the file source's root path.
/// Accessing files using absolute path directly is also possible,
/// but platform-specific and thus generally not recommended.
///
/// @ingroup Filesystem
struct FilesystemDisk : public Filesystem
{
	Allocator *_allocator;
	DynamicString _prefix;

	///
	explicit FilesystemDisk(Allocator &a);

	/// Sets the root path to the given @a prefix.
	/// @note
	/// The @a prefix must be absolute.
	void set_prefix(const char *prefix);

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

	/// @copydoc Filesystem::list_files()
	void list_files(const char *path, Vector<DynamicString> &files) override;

	/// @copydoc Filesystem::absolute_path()
	void absolute_path(DynamicString &os_path, const char *path) override;
};

} // namespace crown
