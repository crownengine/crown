/*
 * Copyright (c) 2012-2016 Daniele Bartolini and individual contributors.
 * License: https://github.com/taylor001/crown/blob/master/LICENSE
 */

#pragma once

#include "dynamic_string.h"
#include "filesystem.h"

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
class FilesystemDisk : public Filesystem
{
	Allocator* _allocator;
	DynamicString _prefix;

public:

	FilesystemDisk(Allocator& a);

	/// Sets the root path to the given @a prefix.
	/// @note
	/// The @a prefix must be absolute.
	void set_prefix(const char* prefix);

	/// @copydoc Filesystem::open()
	File* open(const char* path, FileOpenMode::Enum mode);

	/// @copydoc Filesystem::close()
	void close(File& file);

	/// @copydoc Filesystem::exists()
	bool exists(const char* path);

	/// @copydoc Filesystem::is_directory()
	bool is_directory(const char* path);

	/// @copydoc Filesystem::is_file()
	bool is_file(const char* path);

	/// @copydoc Filesystem::last_modified_time()
	u64 last_modified_time(const char* path);

	/// @copydoc Filesystem::create_directory()
	void create_directory(const char* path);

	/// @copydoc Filesystem::delete_directory()
	void delete_directory(const char* path);

	/// @copydoc Filesystem::create_file()
	void create_file(const char* path);

	/// @copydoc Filesystem::delete_file()
	void delete_file(const char* path);

	/// @copydoc Filesystem::list_files()
	void list_files(const char* path, Vector<DynamicString>& files);

	/// @copydoc Filesystem::get_absolute_path()
	void get_absolute_path(const char* path, DynamicString& os_path);
};

} // namespace crown
