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

#include "MountPoint.h"
#include "HeapAllocator.h"

namespace crown
{

class DiskMountPoint : public MountPoint
{
public:
						DiskMountPoint(const char* root_path);

	File*				open(const char* relative_path, FileOpenMode mode);

	void				close(File* file);

	/// Returns the root path of the mount point
	const char*			root_path() const;

	bool				exists(const char* relative_path);

	/// Returns whether the @a relative_path exists and fills @a info with
	/// with informations about the given @a relative_path path
	bool				get_info(const char* relative_path, MountPointEntry& info);
	
	/// Returns whether @a relative_path is a regular file
	bool				is_file(const char* relative_path);

	/// Returns whether @a relative_path is a directory
	bool				is_dir(const char* relative_path);

	/// Creates a regular file called @a relative_path
	bool				create_file(const char* relative_path);

	/// Creates a directory called @a relative_path
	bool 				create_dir(const char* relative_path);

	/// Deletes a regular file called @a relative_path
	bool				delete_file(const char* relative_path);

	/// Deletes a directory called @a relative_path
	bool 				delete_dir(const char* relative_path);

	/// Returns the os-specific path which @a relative_path refers to.
	/// @note
	/// In general, you typically do not want to use it for normal
	/// file interactions. Prefer using the other methods whenever possible.
	const char*			os_path(const char* relative_path);

protected:

	// Builds the OS-dependent path from base_path and relative_path
	const char*			build_os_path(const char* base_path, const char* relative_path);

protected:

	HeapAllocator		m_allocator;

	char				m_root_path[MAX_PATH_LENGTH];
};

} // namespace crown