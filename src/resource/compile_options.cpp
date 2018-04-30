/*
 * Copyright (c) 2012-2018 Daniele Bartolini and individual contributors.
 * License: https://github.com/dbartolini/crown/blob/master/LICENSE
 */

#include "config.h"
#include "core/containers/array.h"
#include "core/containers/vector.h"
#include "core/filesystem/file.h"
#include "core/filesystem/filesystem.h"
#include "core/filesystem/path.h"
#include "core/guid.h"
#include "core/memory/temp_allocator.h"
#include "core/os.h"
#include "core/strings/dynamic_string.h"
#include "core/strings/string_stream.h"
#include "device/log.h"
#include "resource/compile_options.h"
#include "resource/data_compiler.h"

namespace crown
{
CompileOptions::CompileOptions(DataCompiler& dc, Filesystem& data_filesystem, DynamicString& source_path, Buffer& output, const char* platform)
	: _data_compiler(dc)
	, _data_filesystem(data_filesystem)
	, _source_path(source_path)
	, _output(output)
	, _platform(platform)
	, _dependencies(default_allocator())
{
}

void CompileOptions::error(const char* msg, va_list args)
{
	_data_compiler.error(msg, args);
}

void CompileOptions::error(const char* msg, ...)
{
	va_list args;
	va_start(args, msg);
	error(msg, args);
	va_end(args);
}

const char* CompileOptions::source_path()
{
	return _source_path.c_str();
}

bool CompileOptions::file_exists(const char* path)
{
	TempAllocator256 ta;
	DynamicString source_dir(ta);
	FilesystemDisk fs(ta);

	_data_compiler.source_dir(path, source_dir);
	fs.set_prefix(source_dir.c_str());

	return fs.exists(path);
}

bool CompileOptions::resource_exists(const char* type, const char* name)
{
	TempAllocator1024 ta;
	DynamicString path(ta);
	path += name;
	path += ".";
	path += type;
	return file_exists(path.c_str());
}

Buffer CompileOptions::read_temporary(const char* path)
{
	File* file = _data_filesystem.open(path, FileOpenMode::READ);
	u32 size = file->size();
	Buffer buf(default_allocator());
	array::resize(buf, size);
	file->read(array::begin(buf), size);
	_data_filesystem.close(*file);
	return buf;
}

void CompileOptions::write_temporary(const char* path, const char* data, u32 size)
{
	File* file = _data_filesystem.open(path, FileOpenMode::WRITE);
	file->write(data, size);
	_data_filesystem.close(*file);
}

void CompileOptions::write_temporary(const char* path, const Buffer& data)
{
	write_temporary(path, array::begin(data), array::size(data));
}

///
Buffer CompileOptions::read()
{
	return read(_source_path.c_str());
}

Buffer CompileOptions::read(const char* path)
{
	add_dependency(path);

	TempAllocator256 ta;
	DynamicString source_dir(ta);
	_data_compiler.source_dir(path, source_dir);

	FilesystemDisk source_filesystem(ta);
	source_filesystem.set_prefix(source_dir.c_str());

	File* file = source_filesystem.open(path, FileOpenMode::READ);
	const u32 size = file->size();
	Buffer buf(default_allocator());
	array::resize(buf, size);
	file->read(array::begin(buf), size);
	source_filesystem.close(*file);
	return buf;
}

void CompileOptions::get_absolute_path(const char* path, DynamicString& abs)
{
	TempAllocator256 ta;
	DynamicString source_dir(ta);
	_data_compiler.source_dir(path, source_dir);

	FilesystemDisk source_filesystem(ta);
	source_filesystem.set_prefix(source_dir.c_str());
	source_filesystem.get_absolute_path(path, abs);
}

void CompileOptions::get_temporary_path(const char* suffix, DynamicString& abs)
{
	TempAllocator1024 ta;
	DynamicString str(ta);
	DynamicString prefix(ta);
	guid::to_string(guid::new_guid(), prefix);

	_data_filesystem.get_absolute_path(CROWN_TEMP_DIRECTORY, str);

	path::join(abs, str.c_str(), prefix.c_str());
	abs += '.';
	abs += suffix;
}

void CompileOptions::delete_file(const char* path)
{
	_data_filesystem.delete_file(path);
}

void CompileOptions::write(const void* data, u32 size)
{
	array::push(_output, (const char*)data, size);
}

void CompileOptions::write(const Buffer& data)
{
	array::push(_output, array::begin(data), array::size(data));
}

const char* CompileOptions::platform() const
{
	return _platform;
}

const Vector<DynamicString>& CompileOptions::dependencies() const
{
	return _dependencies;
}

void CompileOptions::add_dependency(const char* path)
{
	TempAllocator256 ta;
	DynamicString dep(ta);
	dep += path;
	vector::push_back(_dependencies, dep);
}

int CompileOptions::run_external_compiler(const char* const* argv, StringStream& output)
{
	return os::execute_process(argv, output);
}

} // namespace crown
