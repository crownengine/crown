/*
 * Copyright (c) 2012-2026 Daniele Bartolini et al.
 * SPDX-License-Identifier: MIT
 */

#pragma once

#include "core/filesystem/filesystem.h"
#include "core/strings/dynamic_string.h"

namespace crown
{
/// Access files over HTTP(S).
/// Files are read-only. Uses byte ranges when available, otherwise temporary storage.
/// HTML5 fallback requires OPFS. Expose Content-Range through CORS to enable range reads.
///
/// @ingroup Filesystem
struct FilesystemHttp : public Filesystem
{
	Allocator *_allocator;
	DynamicString _prefix;

	/// Sets the root URL to @a url.
	FilesystemHttp(Allocator &a, const char *url);

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
