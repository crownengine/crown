/*
 * Copyright (c) 2012-2015 Daniele Bartolini and individual contributors.
 * License: https://github.com/taylor001/crown/blob/master/LICENSE
 */

#pragma once

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
class DiskFilesystem : public Filesystem
{
public:

	/// Sets the root path to the current working directory of
	/// the engine executable.
	DiskFilesystem();

	/// Sets the root path to the given @a prefix.
	/// @note
	/// The @a prefix must be absolute.
	DiskFilesystem(const char* prefix);

	/// Opens the file at the given @a path with the given @a mode.
	File* open(const char* path, FileOpenMode mode);

	/// Closes the given @a file.
	void close(File& file);

	/// Returns whether @a path exists.
	bool exists(const char* path);

	/// Returns true if @a path is a directory.
	bool is_directory(const char* path);

	/// Returns true if @a path is a regular file.
	bool is_file(const char* path);

	/// Creates the directory at the given @a path.
	void create_directory(const char* path);

	/// Deletes the directory at the given @a path.
	void delete_directory(const char* path);

	/// Creates the file at the given @a path.
	void create_file(const char* path);

	/// Deletes the file at the given @a path.
	void delete_file(const char* path);

	/// Returns the relative file names in the given @a path.
	void list_files(const char* path, Vector<DynamicString>& files);

	/// Returns the absolute path of the given @a path based on
	/// the root path of the file source. If @a path is absolute,
	/// the given path is returned.
	void get_absolute_path(const char* path, DynamicString& os_path);

private:

	DynamicString _prefix;
};

} // namespace crown
