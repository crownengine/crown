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

#include "disk_filesystem.h"
#include "string_utils.h"
#include "temp_allocator.h"
#include "disk_file.h"
#include "vector.h"

namespace crown
{

DiskFilesystem::DiskFilesystem()
{
	os::getcwd(_root_path, MAX_PATH_LENGTH);
}

DiskFilesystem::DiskFilesystem(const char* root_path)
{
	CE_ASSERT_NOT_NULL(root_path);
	strncpy(_root_path, root_path, MAX_PATH_LENGTH);
}

File* DiskFilesystem::open(const char* path, FileOpenMode mode)
{
	CE_ASSERT_NOT_NULL(path);

	TempAllocator256 alloc;
	DynamicString abs_path(alloc);
	get_absolute_path(path, abs_path);

	return CE_NEW(default_allocator(), DiskFile)(mode, abs_path.c_str());
}

void DiskFilesystem::close(File* file)
{
	CE_ASSERT_NOT_NULL(file);

	CE_DELETE(default_allocator(), file);
}

bool DiskFilesystem::exists(const char* path)
{
	CE_ASSERT_NOT_NULL(path);

	TempAllocator256 alloc;
	DynamicString abs_path(alloc);
	get_absolute_path(path, abs_path);

	return os::exists(abs_path.c_str());
}

bool DiskFilesystem::is_directory(const char* path)
{
	CE_ASSERT_NOT_NULL(path);

	TempAllocator256 alloc;
	DynamicString abs_path(alloc);
	get_absolute_path(path, abs_path);

	return os::is_directory(abs_path.c_str());
}

bool DiskFilesystem::is_file(const char* path)
{
	CE_ASSERT_NOT_NULL(path);

	TempAllocator256 alloc;
	DynamicString abs_path(alloc);
	get_absolute_path(path, abs_path);

	return os::is_file(abs_path.c_str());
}

void DiskFilesystem::create_directory(const char* path)
{
	CE_ASSERT_NOT_NULL(path);

	TempAllocator256 alloc;
	DynamicString abs_path(alloc);
	get_absolute_path(path, abs_path);

	if (!os::exists(abs_path.c_str()))
		os::create_directory(abs_path.c_str());
}

void DiskFilesystem::delete_directory(const char* path)
{
	CE_ASSERT_NOT_NULL(path);

	TempAllocator256 alloc;
	DynamicString abs_path(alloc);
	get_absolute_path(path, abs_path);

	os::delete_directory(abs_path.c_str());
}

void DiskFilesystem::create_file(const char* path)
{
	CE_ASSERT_NOT_NULL(path);

	TempAllocator256 alloc;
	DynamicString abs_path(alloc);
	get_absolute_path(path, abs_path);

	os::create_file(abs_path.c_str());
}

void DiskFilesystem::delete_file(const char* path)
{
	CE_ASSERT_NOT_NULL(path);

	TempAllocator256 alloc;
	DynamicString abs_path(alloc);
	get_absolute_path(path, abs_path);

	os::delete_file(abs_path.c_str());
}

void DiskFilesystem::list_files(const char* path, Vector<DynamicString>& files)
{
	CE_ASSERT_NOT_NULL(path);

	TempAllocator256 alloc;
	DynamicString abs_path(alloc);
	get_absolute_path(path, abs_path);

	os::list_files(abs_path.c_str(), files);
}

void DiskFilesystem::get_absolute_path(const char* path, DynamicString& os_path)
{
	if (os::is_absolute_path(path))
	{
		os_path = path;
		return;
	}

	os_path += _root_path;
	os_path += PATH_SEPARATOR;
	os_path += path;
}

} // namespace crown
