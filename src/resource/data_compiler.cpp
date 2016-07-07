/*
 * Copyright (c) 2012-2016 Daniele Bartolini and individual contributors.
 * License: https://github.com/taylor001/crown/blob/master/LICENSE
 */

#include "allocator.h"
#include "compile_options.h"
#include "config.h"
#include "console_server.h"
#include "data_compiler.h"
#include "dynamic_string.h"
#include "file.h"
#include "filesystem_disk.h"
#include "log.h"
#include "map.h"
#include "os.h"
#include "path.h"
#include "sjson.h"
#include "sort_map.h"
#include "temp_allocator.h"
#include "vector.h"
#include <setjmp.h>

namespace crown
{
class LineReader
{
	const char* _str;
	const u32 _len;
	u32 _pos;

public:

	LineReader(const char* str)
		: _str(str)
		, _len(strlen32(str))
		, _pos(0)
	{
	}

	void read_line(DynamicString& line)
	{
		const char* s  = &_str[_pos];
		const char* nl = strnl(s);
		_pos += u32(nl - s);
		line.set(s, nl - s);
	}

	bool eof()
	{
		return _str[_pos] == '\0';
	}
};

DataCompiler::DataCompiler()
	: _source_fs(default_allocator())
	, _compilers(default_allocator())
	, _files(default_allocator())
	, _globs(default_allocator())
{
}

void DataCompiler::scan(const char* source_dir)
{
	_source_fs.set_prefix(source_dir);

	TempAllocator512 ta;
	DynamicString ext_tmp(ta);
	DynamicString ext_wav(ta);
	DynamicString ext_ogg(ta);
	DynamicString ext_png(ta);
	DynamicString ext_tga(ta);
	DynamicString ext_dds(ta);
	DynamicString ext_ktx(ta);
	DynamicString ext_pvr(ta);
	DynamicString ext_swn(ta); // VIM swap file.
	DynamicString ext_swo(ta); // VIM swap file.
	DynamicString ext_swp(ta); // VIM swap file.
	DynamicString ext_bak(ta);
	DynamicString ext_all(ta);
	ext_tmp.set("*.tmp", 5);
	ext_wav.set("*.wav", 5);
	ext_ogg.set("*.ogg", 5);
	ext_png.set("*.png", 5);
	ext_tga.set("*.tga", 5);
	ext_dds.set("*.dds", 5);
	ext_ktx.set("*.ktx", 5);
	ext_pvr.set("*.pvr", 5);
	ext_swn.set("*.swn", 5);
	ext_swo.set("*.swo", 5);
	ext_swp.set("*.swp", 5);
	ext_bak.set("*~", 2);
	ext_all.set(".*", 2);

	vector::push_back(_globs, ext_tmp);
	vector::push_back(_globs, ext_wav);
	vector::push_back(_globs, ext_ogg);
	vector::push_back(_globs, ext_png);
	vector::push_back(_globs, ext_tga);
	vector::push_back(_globs, ext_dds);
	vector::push_back(_globs, ext_ktx);
	vector::push_back(_globs, ext_pvr);
	vector::push_back(_globs, ext_swn);
	vector::push_back(_globs, ext_swo);
	vector::push_back(_globs, ext_swp);
	vector::push_back(_globs, ext_bak);
	vector::push_back(_globs, ext_all);

	if (_source_fs.exists(CROWN_BUNDLEIGNORE))
	{
		File& file = *_source_fs.open(CROWN_BUNDLEIGNORE, FileOpenMode::READ);
		const u32 size = file.size();
		char* data = (char*)default_allocator().allocate(size + 1);
		file.read(data, size);
		data[size] = '\0';
		_source_fs.close(file);

		LineReader lr(data);

		while (!lr.eof())
		{
			TempAllocator512 ta;
			DynamicString line(ta);
			lr.read_line(line);

			line.trim();

			if (line.empty() || line.has_prefix("#"))
				continue;

			vector::push_back(_globs, line);
		}

		default_allocator().deallocate(data);
	}

	scan_source_dir("");
}

bool DataCompiler::compile(FilesystemDisk& bundle_fs, const char* type, const char* name, const char* platform)
{
	StringId64 _type(type);
	StringId64 _name(name);

	TempAllocator1024 ta;
	DynamicString path(ta);
	DynamicString src_path(ta);

	// Build source file path
	src_path += name;
	src_path += '.';
	src_path += type;

	// Build compiled file path
	DynamicString type_str(ta);
	DynamicString name_str(ta);
	_type.to_string(type_str);
	_name.to_string(name_str);

	DynamicString dst_path(ta);
	dst_path += type_str;
	dst_path += '-';
	dst_path += name_str;

	path::join(CROWN_DATA_DIRECTORY, dst_path.c_str(), path);

	logi("%s <= %s", dst_path.c_str(), src_path.c_str());

	bool success = true;
	jmp_buf buf;

	Buffer output(default_allocator());
	array::reserve(output, 4*1024*1024);

	if (!setjmp(buf))
	{
		CompileOptions opts(_source_fs, bundle_fs, output, platform, &buf);
		compile(_type, src_path.c_str(), opts);
		File* outf = bundle_fs.open(path.c_str(), FileOpenMode::WRITE);
		u32 size = array::size(output);
		u32 written = outf->write(array::begin(output), size);
		bundle_fs.close(*outf);
		success = size == written;
	}
	else
	{
		success = false;
	}

	return success;
}

bool DataCompiler::compile(const char* bundle_dir, const char* platform)
{
	// Create bundle dir if necessary
	FilesystemDisk bundle_fs(default_allocator());
	bundle_fs.set_prefix(bundle_dir);
	bundle_fs.create_directory("");

	if (!bundle_fs.exists(CROWN_DATA_DIRECTORY))
		bundle_fs.create_directory(CROWN_DATA_DIRECTORY);

	if (!bundle_fs.exists(CROWN_TEMP_DIRECTORY))
		bundle_fs.create_directory(CROWN_TEMP_DIRECTORY);

	// Compile all changed resources
	for (u32 i = 0; i < vector::size(_files); ++i)
	{
		const char* filename = _files[i].c_str();

		bool ignore = false;
		for (u32 gg = 0; gg < vector::size(_globs); ++gg)
		{
			const char* glob = _globs[gg].c_str();

			if (wildcmp(glob, filename))
			{
				ignore = true;
				break;
			}
		}

		const char* type = path::extension(filename);

		if (ignore || type == NULL)
			continue;

		char name[256];
		const u32 size = u32(type - filename - 1);
		strncpy(name, filename, size);
		name[size] = '\0';

		if (!compile(bundle_fs, type, name, platform))
			return false;
	}

	return true;
}

void DataCompiler::register_compiler(StringId64 type, u32 version, CompileFunction compiler)
{
	CE_ASSERT(!sort_map::has(_compilers, type), "Type already registered");
	CE_ASSERT_NOT_NULL(compiler);

	ResourceTypeData rtd;
	rtd.version = version;
	rtd.compiler = compiler;

	sort_map::set(_compilers, type, rtd);
	sort_map::sort(_compilers);
}

void DataCompiler::compile(StringId64 type, const char* path, CompileOptions& opts)
{
	CE_ASSERT(sort_map::has(_compilers, type), "Compiler not found");

	sort_map::get(_compilers, type, ResourceTypeData()).compiler(path, opts);
}

u32 DataCompiler::version(StringId64 type)
{
	CE_ASSERT(sort_map::has(_compilers, type), "Compiler not found");

	return sort_map::get(_compilers, type, ResourceTypeData()).version;
}

void DataCompiler::scan_source_dir(const char* cur_dir)
{
	Vector<DynamicString> my_files(default_allocator());
	_source_fs.list_files(cur_dir, my_files);

	for (u32 i = 0; i < vector::size(my_files); ++i)
	{
		TempAllocator512 ta;
		DynamicString file_i(ta);

		if (strcmp(cur_dir, "") != 0)
		{
			file_i += cur_dir;
			file_i += '/';
		}
		file_i += my_files[i];

		if (_source_fs.is_directory(file_i.c_str()))
		{
			DataCompiler::scan_source_dir(file_i.c_str());
		}
		else // Assume a regular file
		{
			vector::push_back(_files, file_i);
		}
	}
}

} // namespace crown
