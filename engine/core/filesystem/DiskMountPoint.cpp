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

#include "DiskMountPoint.h"
#include "Assert.h"
#include "DiskFile.h"
#include "StringUtils.h"
#include "Allocator.h"

namespace crown
{

//-----------------------------------------------------------------------------
DiskMountPoint::DiskMountPoint() : MountPoint(DISK_TYPE)
{
}

//-----------------------------------------------------------------------------
File* DiskMountPoint::open(const char* relative_path, FileOpenMode mode)
{
	CE_ASSERT(exists(relative_path), "File does not exist: %s", relative_path);
	CE_ASSERT(is_file(relative_path), "File is not a regular file: %s", relative_path);

	return CE_NEW(default_allocator(), DiskFile)(mode, os_path(relative_path));
}

//-----------------------------------------------------------------------------
void DiskMountPoint::close(File* file)
{
	CE_DELETE(default_allocator(), file);
}

void DiskMountPoint::set_root_path(const char* root_path)
{
	CE_ASSERT(root_path != NULL, "Root path must be != NULL");
	CE_ASSERT(os::is_absolute_path(root_path), "Root path must be absolute");

	string::strncpy(m_root_path, root_path, MAX_PATH_LENGTH);
}


//-----------------------------------------------------------------------------
const char*	DiskMountPoint::root_path() const
{
	return m_root_path;
}

//-----------------------------------------------------------------------------
bool DiskMountPoint::exists(const char* relative_path)
{
	MountPointEntry info;

	return get_info(relative_path, info);	
}
//-----------------------------------------------------------------------------
bool DiskMountPoint::get_info(const char* relative_path, MountPointEntry& info)
{
	// Entering OS-DEPENDENT-PATH-MODE
	// (i.e. os_path is of the form: C:\foo\relative_path or /foo/relative_path)

	const char* os_path = build_os_path(m_root_path, relative_path);

	Log::d("path : %s", os_path);

	string::strncpy(info.os_path, os_path, MAX_PATH_LENGTH);
	string::strncpy(info.relative_path, relative_path, MAX_PATH_LENGTH);

	if (os::is_reg(os_path))
	{
		info.type = MountPointEntry::FILE;
		return true;
	}
	else if (os::is_dir(os_path))
	{
		info.type = MountPointEntry::DIRECTORY;
		return true;
	}
	
	info.type = MountPointEntry::UNKNOWN;

	return false;
}

//-----------------------------------------------------------------------------
bool DiskMountPoint::is_file(const char* relative_path)
{
	MountPointEntry info;

	if (get_info(relative_path, info))
	{
		return info.type == MountPointEntry::FILE;
	}

	return false;
}

//-----------------------------------------------------------------------------
bool DiskMountPoint::is_dir(const char* relative_path)
{
	MountPointEntry info;

	if (get_info(relative_path, info))
	{
		return info.type == MountPointEntry::DIRECTORY;
	}

	return false;
}

//-----------------------------------------------------------------------------
bool DiskMountPoint::create_file(const char* relative_path)
{
	const char* os_path = build_os_path(m_root_path, relative_path);

	return os::mknod(os_path);
}

//-----------------------------------------------------------------------------
bool DiskMountPoint::create_dir(const char* relative_path)
{
	const char* os_path = build_os_path(m_root_path, relative_path);

	return os::mkdir(os_path);
}

//-----------------------------------------------------------------------------
bool DiskMountPoint::delete_file(const char* relative_path)
{
	const char* os_path = build_os_path(m_root_path, relative_path);

	return os::unlink(os_path);
}

//-----------------------------------------------------------------------------
bool DiskMountPoint::delete_dir(const char* relative_path)
{
	const char* os_path = build_os_path(m_root_path, relative_path);

	return os::rmdir(os_path);
}

//-----------------------------------------------------------------------------
const char* DiskMountPoint::os_path(const char* relative_path)
{
	static char os_path[MAX_PATH_LENGTH];

	MountPointEntry entry;

	get_info(relative_path, entry);

	string::strncpy(os_path, entry.os_path, MAX_PATH_LENGTH);

	return os_path;
}

//-----------------------------------------------------------------------------
const char* DiskMountPoint::build_os_path(const char* base_path, const char* relative_path)
{
	static char os_path[MAX_PATH_LENGTH];

	string::strncpy(os_path, base_path, MAX_PATH_LENGTH);

	size_t base_path_len = string::strlen(base_path);

	os_path[base_path_len] = PATH_SEPARATOR;
	os_path[base_path_len + 1] = '\0';

	string::strcat(os_path, relative_path);

	// FIXME FIXME FIXME Replace Crown-specific path separator with OS-specific one
	for (size_t j = 0; j < string::strlen(os_path); j++)
	{
		if (os_path[j] == '/')
		{
			os_path[j] = PATH_SEPARATOR;
		}
	}

	return os_path;
}

} // namespace crown