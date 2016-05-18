/*
 * Copyright (c) 2012-2016 Daniele Bartolini and individual contributors.
 * License: https://github.com/taylor001/crown/blob/master/LICENSE
 */

#include "allocator.h"
#include "bundle_compiler.h"
#include "compile_options.h"
#include "config.h"
#include "console_server.h"
#include "disk_filesystem.h"
#include "dynamic_string.h"
#include "file.h"
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

BundleCompiler::BundleCompiler()
	: _source_fs(default_allocator())
	, _compilers(default_allocator())
	, _files(default_allocator())
	, _globs(default_allocator())
{
}

void BundleCompiler::scan(const char* source_dir)
{
	_source_fs.set_prefix(source_dir);

	TempAllocator512 ta;
	vector::push_back(_globs, DynamicString("*.tmp", ta));
	vector::push_back(_globs, DynamicString("*.wav", ta));
	vector::push_back(_globs, DynamicString("*.ogg", ta));
	vector::push_back(_globs, DynamicString("*.png", ta));
	vector::push_back(_globs, DynamicString("*.tga", ta));
	vector::push_back(_globs, DynamicString("*.dds", ta));
	vector::push_back(_globs, DynamicString("*.ktx", ta));
	vector::push_back(_globs, DynamicString("*.pvr", ta));
	vector::push_back(_globs, DynamicString("*.swn", ta)); // VIM swap file.
	vector::push_back(_globs, DynamicString("*.swo", ta)); // VIM swap file.
	vector::push_back(_globs, DynamicString("*.swp", ta)); // VIM swap file.
	vector::push_back(_globs, DynamicString("*~", ta));
	vector::push_back(_globs, DynamicString(".*", ta));

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

bool BundleCompiler::compile(DiskFilesystem& bundle_fs, const char* type, const char* name, const char* platform)
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

	CE_LOGI("%s <= %s", dst_path.c_str(), src_path.c_str());

	bool success = true;
	jmp_buf buf;

	Buffer output(default_allocator());
	array::reserve(output, 4*1024*1024);

	if (!setjmp(buf))
	{
		CompileOptions opts(_source_fs, output, platform, &buf);
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

bool BundleCompiler::compile(const char* bundle_dir, const char* platform)
{
	// Create bundle dir if necessary
	DiskFilesystem bundle_fs(default_allocator());
	bundle_fs.set_prefix(bundle_dir);
	bundle_fs.create_directory("");

	if (!bundle_fs.exists(CROWN_DATA_DIRECTORY))
		bundle_fs.create_directory(CROWN_DATA_DIRECTORY);

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

void BundleCompiler::register_compiler(StringId64 type, u32 version, CompileFunction compiler)
{
	CE_ASSERT(!sort_map::has(_compilers, type), "Type already registered");
	CE_ASSERT_NOT_NULL(compiler);

	ResourceTypeData rtd;
	rtd.version = version;
	rtd.compiler = compiler;

	sort_map::set(_compilers, type, rtd);
	sort_map::sort(_compilers);
}

void BundleCompiler::compile(StringId64 type, const char* path, CompileOptions& opts)
{
	CE_ASSERT(sort_map::has(_compilers, type), "Compiler not found");

	sort_map::get(_compilers, type, ResourceTypeData()).compiler(path, opts);
}

u32 BundleCompiler::version(StringId64 type)
{
	CE_ASSERT(sort_map::has(_compilers, type), "Compiler not found");

	return sort_map::get(_compilers, type, ResourceTypeData()).version;
}

void BundleCompiler::scan_source_dir(const char* cur_dir)
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
			BundleCompiler::scan_source_dir(file_i.c_str());
		}
		else // Assume a regular file
		{
			vector::push_back(_files, file_i);
		}
	}
}

} // namespace crown
