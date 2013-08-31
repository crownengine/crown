/*
Copyright (c) 2013 Daniele Bartolini, Michele Rossi
Copyright (c) 2012 Daniele Bartolini, Simone Boscaratto

Permission is hereby granted, free of charge, to any person
obtaining a copy of this software and associated documentation
files (the "Software"), to deal in the Software without
restriction, including without limitation the rights to use,
copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the
Software is furnished to do so, subject to the following
conditions:

The above copyright notice and this permission notice shall be
included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
OTHER DEALINGS IN THE SOFTWARE.
*/

#pragma once

#include "Filesystem.h"
#include "OS.h" // For MAX_PATH_LENGTH

namespace crown
{

/// Access files on disk.
/// All the file paths can be either relative or absolute.
/// When a relative path is given, it is automatically translated
/// to its absolute counterpart based on the file source's root path.
/// Accessing files using absolute path directly is also possible,
/// but platform-specific and thus generally not recommended.
class DiskFilesystem : public Filesystem
{
public:

	/// Sets the root path to the current working directory of
	/// the engine executable.
	DiskFilesystem();

	/// Sets the root path to the given @a root_path.
	/// @note
	/// The @a root_path must be absolute.
	DiskFilesystem(const char* root_path);

	/// Opens the file at the given @a path with the given @a mode.
	File* open(const char* path, FileOpenMode mode);

	/// Closes the given @a file.
	void close(File* file);

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

	char m_root_path[MAX_PATH_LENGTH];
};

} // namespace crown
