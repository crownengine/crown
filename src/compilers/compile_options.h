/*
 * Copyright (c) 2012-2015 Daniele Bartolini and individual contributors.
 * License: https://github.com/taylor001/crown/blob/master/LICENSE
 */

#pragma once

#include "filesystem.h"
#include "reader_writer.h"
#include "log.h"
#include <setjmp.h>

#define RESOURCE_COMPILER_ASSERT(condition, opts, msg, ...) do { if (!(condition))\
	{ opts.error(msg, ##__VA_ARGS__); } } while(0)

namespace crown
{

struct CompileOptions
{
	CompileOptions(Filesystem& fs, File* out, const char* platform, jmp_buf* buf)
		: _fs(fs)
		, _bw(*out)
		, _platform(platform)
		, _jmpbuf(buf)
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

	Buffer read(const char* path)
	{
		File* file = _fs.open(path, FOM_READ);
		uint32_t size = file->size();
		Buffer buf(default_allocator());
		array::resize(buf, size);
		file->read(array::begin(buf), size);
		_fs.close(file);
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

	BinaryWriter& write(const void* data, uint32_t size)
	{
		_bw.write(data, size);
		return _bw;
	}

	template <typename T>
	BinaryWriter& write(const T& data)
	{
		_bw.write(data);
		return _bw;
	}

	BinaryWriter& write(const Buffer& data)
	{
		if (array::size(data))
			_bw.write(array::begin(data), array::size(data));

		return _bw;
	}

	const char* platform() const
	{
		return _platform;
	}

	Filesystem& _fs;
	BinaryWriter _bw;
	const char* _platform;
	jmp_buf* _jmpbuf;
};

} // namespace crown
