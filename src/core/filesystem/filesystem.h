/*
 * Copyright (c) 2012-2016 Daniele Bartolini and individual contributors.
 * License: https://github.com/taylor001/crown/blob/master/LICENSE
 */

#pragma once

#include "container_types.h"
#include "filesystem_types.h"
#include "string_types.h"

namespace crown
{
/// Provides a platform-independent way to access files and directories
/// on the host filesystem.
///
/// @ingroup Filesystem
class Filesystem
{
public:

	Filesystem() {};
	virtual ~Filesystem() {};

	/// Opens the file at the given @a path with the given @a mode.
	virtual File* open(const char* path, FileOpenMode::Enum mode) = 0;

	/// Closes the given @a file.
	virtual void close(File& file) = 0;

	/// Returns whether @a path exists.
	virtual bool exists(const char* path) = 0;

	/// Returns true if @a path is a directory.
	virtual bool is_directory(const char* path) = 0;

	/// Returns true if @a path is a regular file.
	virtual bool is_file(const char* path) = 0;

	/// Returns the time of last modify operaton to @a path.
	virtual u64 last_modified_time(const char* path) = 0;

	/// Creates the directory at the given @a path.
	virtual void create_directory(const char* path) = 0;

	/// Deletes the directory at the given @a path.
	virtual void delete_directory(const char* path) = 0;

	/// Creates the file at the given @a path.
	virtual void create_file(const char* path) = 0;

	/// Deletes the file at the given @a path.
	virtual void delete_file(const char* path) = 0;

	/// Returns the relative file names in the given @a path.
	virtual void list_files(const char* path, Vector<DynamicString>& files) = 0;

	/// Returns the absolute path of the given @a path based on
	/// the root path of the file source. If @a path is absolute,
	/// the given path is returned.
	virtual void get_absolute_path(const char* path, DynamicString& os_path) = 0;

private:

	// Disable copying
	Filesystem(const Filesystem&);
	Filesystem& operator=(const Filesystem&);
};

} // namespace crown
