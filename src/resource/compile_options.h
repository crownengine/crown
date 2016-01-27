/*
 * Copyright (c) 2012-2016 Daniele Bartolini and individual contributors.
 * License: https://github.com/taylor001/crown/blob/master/LICENSE
 */

#pragma once

#include "filesystem.h"
#include "file.h"
#include "log.h"
#include "vector.h"
#include "temp_allocator.h"
#include "path.h"
#include <setjmp.h>

#define RESOURCE_COMPILER_ASSERT(condition, opts, msg, ...) do { if (!(condition))\
	{ opts.error(msg, ##__VA_ARGS__); } } while(0)

namespace crown
{
struct CompileOptions
{
	CompileOptions(Filesystem& fs, Buffer& output, const char* platform, jmp_buf* buf)
		: _fs(fs)
		, _output(output)
		, _platform(platform)
		, _jmpbuf(buf)
		, _dependencies(default_allocator())
	{
	}

	void error(const char* msg, va_list args)
	{
		CE_LOGEV(msg, args);
		longjmp(*_jmpbuf, 1);
	}

	void error(const char* msg, ...)
	{
		va_list args;
		va_start(args, msg);
		error(msg, args);
		va_end(args);
	}

	bool file_exists(const char* path)
	{
		return _fs.exists(path);
	}

	bool resource_exists(const char* type, const char* name)
	{
		TempAllocator1024 ta;
		DynamicString path(name, ta);
		path += ".";
		path += type;
		return file_exists(path.c_str());
	}

	Buffer read(const char* path)
	{
		add_dependency(path);

		File* file = _fs.open(path, FileOpenMode::READ);
		u32 size = file->size();
		Buffer buf(default_allocator());
		array::resize(buf, size);
		file->read(array::begin(buf), size);
		_fs.close(*file);
		return buf;
	}

	void get_absolute_path(const char* path, DynamicString& abs)
	{
		_fs.get_absolute_path(path, abs);
	}

	void delete_file(const char* path)
	{
		_fs.delete_file(path);
	}

	void write(const void* data, u32 size)
	{
		array::push(_output, (const char*)data, size);
	}

	template <typename T>
	void write(const T& data)
	{
		write(&data, sizeof(data));
	}

	void write(const Buffer& data)
	{
		array::push(_output, array::begin(data), array::size(data));
	}

	const char* platform() const
	{
		return _platform;
	}

	const Vector<DynamicString>& dependencies() const
	{
		return _dependencies;
	}

	void add_dependency(const char* path)
	{
		TempAllocator256 ta;
		DynamicString dep(path, ta);
		vector::push_back(_dependencies, dep);
	}

	Filesystem& _fs;
	Buffer& _output;
	const char* _platform;
	jmp_buf* _jmpbuf;
	Vector<DynamicString> _dependencies;
};

} // namespace crown
