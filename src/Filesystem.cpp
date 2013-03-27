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
Filesystem::Filesystem(const char* root_path) :
	m_root_path(root_path)
{
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
	static char os_path[1024];

	size_t i = 0;

	while (*base_path != '\0')
	{
		os_path[i++] = *base_path;
		base_path++;
	}

	os_path[i++] = '/';

	while (*relative_path != '\0')
	{
		os_path[i++] = *relative_path;
		relative_path++;
	}

	os_path[i] = '\0';

	// Replace Crown-specific path separator with OS-speficic one
	for (size_t j = 0; j < i; j++)
	{
		if (os_path[j] == '/')
		{
			os_path[j] = os::PATH_SEPARATOR;
		}
	}

	return os_path;
}

//-----------------------------------------------------------------------------
bool Filesystem::get_info(const char* base_path, const char* relative_path, FilesystemEntry& info)
{
	// Entering OS-DEPENDENT-PATH-MODE
	// (i.e. os_path is of the form: C:\babbeo\relative_path or /babbeo/relative_path)

	const char* os_path = build_os_path(base_path, relative_path);
	
	string::strncpy(info.os_path, os_path, 512);
	string::strncpy(info.relative_path, relative_path, 512);

	if (os::is_reg(os_path))
	{
		info.type = FilesystemEntry::FILE;
		return true;
	}

	if (os::is_dir(os_path))
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

	return get_info(m_root_path, relative_path, dummy);
}

//-----------------------------------------------------------------------------
bool Filesystem::is_file(const char* relative_path)
{
	FilesystemEntry info;

	if (get_info(m_root_path, relative_path, info))
	{
		return info.type == FilesystemEntry::FILE;
	}

	return false;
}

//-----------------------------------------------------------------------------
bool Filesystem::is_dir(const char* relative_path)
{
	FilesystemEntry info;

	if (get_info(m_root_path, relative_path, info))
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
Stream* Filesystem::open(const char* relative_path, StreamOpenMode mode)
{
	FilesystemEntry info;
	Stream* stream;

	get_info(m_root_path, relative_path, info);

	Log::D("Filesystem::OpenStream: Found %s", info.os_path);

	stream = new FileStream(mode, info.os_path);

	return stream;
}

//-----------------------------------------------------------------------------
void Filesystem::close(Stream* stream)
{
	delete stream;
}

} // namespace crown

