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

namespace crown
{

/// Access files on Android's assets folder.
/// The assets folder is read-only and all the paths are relative.
class ApkFilesystem : public Filesystem
{
public:

	ApkFilesystem();

	/// @copydoc Filesystem::open()
	/// @note
	/// @a mode can only be FOM_READ
	File* open(const char* rel_path, FileOpenMode mode);

	/// @copydoc Filesystem::close()
	void close(File* file);

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
};

} // namespace crown
