/*
 * Copyright (c) 2012-2025 Daniele Bartolini et al.
 * SPDX-License-Identifier: MIT
 */

#include "config.h"

#if CROWN_CAN_COMPILE
#include "core/containers/array.inl"
#include "core/containers/hash_map.inl"
#include "core/containers/vector.inl"
#include "core/filesystem/file.h"
#include "core/filesystem/filesystem.h"
#include "core/filesystem/path.h"
#include "core/filesystem/reader_writer.inl"
#include "core/guid.h"
#include "core/json/sjson.h"
#include "core/memory/temp_allocator.inl"
#include "core/option.inl"
#include "core/os.h"
#include "core/process.h"
#include "core/strings/dynamic_string.inl"
#include "core/strings/string_stream.inl"
#include "device/log.h"
#include "resource/compile_options.inl"
#include "resource/data_compiler.h"

namespace crown
{
static const char *s_platforms[] =
{
	"android", // Platform::ANDROID
	"android", // Platform::ANDROID_ARM64
	"html5",   // Platform::HTML5
	"linux",   // Platform::LINUX
	"windows"  // Platform::WINDOWS
};
CE_STATIC_ASSERT(countof(s_platforms) == Platform::COUNT);

static void sjson_error(const char *msg, void *user_data)
{
	CompileOptions *opts = (CompileOptions *)user_data;
	opts->error("%s", msg);
	opts->_sjson_error = true;
}

CompileOptions::CompileOptions(File &output
	, HashMap<DynamicString, u32> &new_dependencies
	, HashMap<DynamicString, u32> &new_requirements
	, DataCompiler &dc
	, Filesystem &output_filesystem
	, Filesystem &data_filesystem
	, ResourceId res_id
	, const DynamicString &source_path
	, Platform::Enum platform
	, bool bundle
	)
	: _file(output)
	, _binary_writer(_file)
	, _new_dependencies(new_dependencies)
	, _new_requirements(new_requirements)
	, _new_requirement_globs(default_allocator())
	, _data_compiler(dc)
	, _output_filesystem(output_filesystem)
	, _data_filesystem(data_filesystem)
	, _source_path(source_path)
	, _platform(platform)
	, _resource_id(res_id)
	, _bundle(bundle)
	, _server(_data_compiler._options->_server)
	, _sjson_error(false)
{
	sjson::set_error_callback(sjson_error, this);
}

void CompileOptions::error(const char *msg, va_list args)
{
	_data_compiler.error(msg, args);
}

void CompileOptions::error(const char *msg, ...)
{
	va_list args;
	va_start(args, msg);
	error(msg, args);
	va_end(args);
}

const char *CompileOptions::source_path()
{
	return _source_path.c_str();
}

bool CompileOptions::file_exists(const char *path)
{
	TempAllocator256 ta;
	DynamicString source_dir(ta);
	FilesystemDisk fs(ta);

	_data_compiler.source_dir(path, source_dir);
	fs.set_prefix(source_dir.c_str());

	return fs.exists(path);
}

bool CompileOptions::resource_exists(const char *type, const char *name)
{
	TempAllocator1024 ta;
	DynamicString path(ta);
	path += name;
	path += ".";
	path += type;
	return file_exists(path.c_str());
}

Buffer CompileOptions::read_all(File *file)
{
	const u32 size = file->size();
	Buffer buf(default_allocator());
	array::resize(buf, size);
	if (size != 0)
		file->read(array::begin(buf), size);
	return buf;
}

Buffer CompileOptions::read_temporary(const char *path)
{
	Buffer buf(default_allocator());
	File *file = _output_filesystem.open(path, FileOpenMode::READ);
	if (file->is_open())
		buf = read_all(file);
	_output_filesystem.close(*file);
	return buf;
}

void CompileOptions::write_temporary(const char *path, const char *data, u32 size)
{
	File *file = _output_filesystem.open(path, FileOpenMode::WRITE);
	if (file->is_open())
		file->write(data, size);
	_output_filesystem.close(*file);
}

void CompileOptions::write_temporary(const char *path, const Buffer &data)
{
	write_temporary(path, array::begin(data), array::size(data));
}

Buffer CompileOptions::read(const char *path)
{
	fake_read(path);

	TempAllocator256 ta;
	DynamicString source_dir(ta);
	_data_compiler.source_dir(path, source_dir);

	FilesystemDisk source_filesystem(ta);
	source_filesystem.set_prefix(source_dir.c_str());

	Buffer buf(default_allocator());
	File *file = source_filesystem.open(path, FileOpenMode::READ);
	if (file->is_open())
		buf = read_all(file);
	source_filesystem.close(*file);
	return buf;
}

Buffer CompileOptions::read()
{
	return read(_source_path.c_str());
}

void CompileOptions::fake_read(const char *path)
{
	TempAllocator256 ta;
	DynamicString path_str(ta);
	path_str = path;

	hash_map::set(_new_dependencies, path_str, 0u);
}

void CompileOptions::add_requirement(const char *type, const char *name)
{
	TempAllocator256 ta;
	DynamicString path(ta);
	path  = name;
	path += ".";
	path += type;

	hash_map::set(_new_requirements, path, 0u);
}

void CompileOptions::add_requirement_glob(const char *glob)
{
	TempAllocator256 ta;
	DynamicString str(ta);
	str = glob;

	vector::push_back(_new_requirement_globs, str);
}

void CompileOptions::absolute_path(DynamicString &abs, const char *path)
{
	TempAllocator256 ta;
	DynamicString source_dir(ta);
	_data_compiler.source_dir(path, source_dir);

	FilesystemDisk source_filesystem(ta);
	source_filesystem.set_prefix(source_dir.c_str());
	source_filesystem.absolute_path(abs, path);
}

void CompileOptions::temporary_path(DynamicString &abs, const char *suffix)
{
	TempAllocator1024 ta;
	DynamicString str(ta);
	DynamicString prefix(ta);
	prefix.from_guid(guid::new_guid());

	_output_filesystem.absolute_path(str, CROWN_TEMP_DIRECTORY);

	path::join(abs, str.c_str(), prefix.c_str());
	abs += '.';
	abs += suffix;
}

DeleteResult CompileOptions::delete_file(const char *path)
{
	return _output_filesystem.delete_file(path);
}

void CompileOptions::align(const u32 align)
{
	_binary_writer.align(align);
}

void CompileOptions::write(const void *data, u32 size)
{
	_binary_writer.write(data, size);
}

void CompileOptions::write(const Buffer &data)
{
	write(array::begin(data), array::size(data));
}

const char *CompileOptions::exe_path(const char * const *paths, u32 num)
{
	for (u32 ii = 0; ii < num; ++ii) {
		if (os::access(paths[ii], AccessFlags::EXECUTE) == 0)
			return paths[ii];
	}

	return NULL;
}

void CompileOptions::read_output(StringStream &output, Process &pr)
{
	u32 nbr = 0;
	char msg[512];
	while (pr.read(&nbr, msg, sizeof(msg) - 1) != NULL) {
		msg[nbr] = '\0';
		output << msg;
	}
}

const char *CompileOptions::platform_name()
{
	return s_platforms[_platform];
}

} // namespace crown

#endif // if CROWN_CAN_COMPILE
