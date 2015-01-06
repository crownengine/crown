/*
 * Copyright (c) 2012-2015 Daniele Bartolini and individual contributors.
 * License: https://github.com/taylor001/crown/blob/master/LICENSE
 */

#include "disk_filesystem.h"
#include "string_utils.h"
#include "temp_allocator.h"
#include "disk_file.h"
#include "vector.h"
#include "path.h"
#include "os.h"

namespace crown
{

DiskFilesystem::DiskFilesystem()
{
	char buf[512];
	os::getcwd(buf, sizeof(buf));
	_prefix = buf;
}

DiskFilesystem::DiskFilesystem(const char* prefix)
	: _prefix(prefix)
{
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
	if (path::is_absolute_path(path))
	{
		os_path = path;
		return;
	}

	path::join(_prefix.c_str(), path, os_path);
}

} // namespace crown
