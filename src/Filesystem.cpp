/*
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

#include "Filesystem.h"
#include "Log.h"
#include "OS.h"
#include "FileStream.h"

namespace crown
{

//-----------------------------------------------------------------------------
Filesystem::Filesystem(const char* root_path)
{
	CE_ASSERT(root_path != NULL, "Root path must be != NULL");
	CE_ASSERT(os::is_absolute_path(root_path), "Root path must be absolute");

	string::strncpy(m_root_path, root_path, os::MAX_PATH_LENGTH);
}

//-----------------------------------------------------------------------------
Filesystem::~Filesystem()
{
}

//-----------------------------------------------------------------------------
const char* Filesystem::root_path() const
{
	return m_root_path;
}

//-----------------------------------------------------------------------------
const char* Filesystem::build_os_path(const char* base_path, const char* relative_path)
{
	static char os_path[os::MAX_PATH_LENGTH];

	string::strncpy(os_path, base_path, os::MAX_PATH_LENGTH);

	size_t base_path_len = string::strlen(base_path);

	os_path[base_path_len] = os::PATH_SEPARATOR;

	string::strncpy(&os_path[base_path_len + 1], relative_path, os::MAX_PATH_LENGTH);

	// FIXME FIXME FIXME Replace Crown-specific path separator with OS-speficic one
	for (size_t j = 0; j < string::strlen(os_path); j++)
	{
		if (os_path[j] == '/')
		{
			os_path[j] = os::PATH_SEPARATOR;
		}
	}

	return os_path;
}

//-----------------------------------------------------------------------------
bool Filesystem::get_info(const char* relative_path, FilesystemEntry& info)
{
	// Entering OS-DEPENDENT-PATH-MODE
	// (i.e. os_path is of the form: C:\foo\relative_path or /foo/relative_path)

	const char* os_path = build_os_path(m_root_path, relative_path);
	
	string::strncpy(info.os_path, os_path, os::MAX_PATH_LENGTH);
	string::strncpy(info.relative_path, relative_path, os::MAX_PATH_LENGTH);

	if (os::is_reg(os_path))
	{
		info.type = FilesystemEntry::FILE;
		return true;
	}
	else if (os::is_dir(os_path))
	{
		info.type = FilesystemEntry::DIRECTORY;
		return true;
	}
	
	info.type = FilesystemEntry::UNKNOWN;

	return false;
}

//-----------------------------------------------------------------------------
bool Filesystem::exists(const char* relative_path)
{
	FilesystemEntry dummy;

	return get_info(relative_path, dummy);
}

//-----------------------------------------------------------------------------
bool Filesystem::is_file(const char* relative_path)
{
	FilesystemEntry info;

	if (get_info(relative_path, info))
	{
		return info.type == FilesystemEntry::FILE;
	}

	return false;
}

//-----------------------------------------------------------------------------
bool Filesystem::is_dir(const char* relative_path)
{
	FilesystemEntry info;

	if (get_info(relative_path, info))
	{
		return info.type == FilesystemEntry::DIRECTORY;
	}

	return false;
}

//-----------------------------------------------------------------------------
bool Filesystem::create_file(const char* relative_path)
{
	const char* os_path = build_os_path(m_root_path, relative_path);

	return os::mknod(os_path);
}

//-----------------------------------------------------------------------------
bool Filesystem::create_dir(const char* relative_path)
{
	const char* os_path = build_os_path(m_root_path, relative_path);

	return os::mkdir(os_path);
}

//-----------------------------------------------------------------------------
bool Filesystem::delete_file(const char* relative_path)
{
	const char* os_path = build_os_path(m_root_path, relative_path);

	return os::unlink(os_path);
}

//-----------------------------------------------------------------------------
bool Filesystem::delete_dir(const char* relative_path)
{
	const char* os_path = build_os_path(m_root_path, relative_path);

	return os::rmdir(os_path);
}

//-----------------------------------------------------------------------------
const char* Filesystem::os_path(const char* relative_path)
{
	static char os_path[os::MAX_PATH_LENGTH];

	FilesystemEntry entry;

	get_info(relative_path, entry);

	string::strncpy(os_path, entry.os_path, os::MAX_PATH_LENGTH);

	return os_path;
}

//-----------------------------------------------------------------------------
FileStream* Filesystem::open(const char* relative_path, StreamOpenMode mode)
{
	FilesystemEntry info;

	CE_ASSERT(get_info(relative_path, info), "File does not exist: %s", relative_path);
	CE_ASSERT(info.type == FilesystemEntry::FILE, "File is not a regular file: %s", relative_path);

	return new FileStream(mode, info.os_path);
}

//-----------------------------------------------------------------------------
void Filesystem::close(FileStream* stream)
{
	delete stream;
}

} // namespace crown

