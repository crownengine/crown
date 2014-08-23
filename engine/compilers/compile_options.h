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

#pragma once

#include "filesystem.h"
#include "reader_writer.h"

namespace crown
{

typedef Array<char> Buffer;

struct CompileOptions
{
	CompileOptions(Filesystem& fs, File* out, const char* platform)
		: _fs(fs)
		, _bw(*out)
		, _platform(platform)
	{
	}

	Buffer read(const char* path)
	{
		File* file = _fs.open(path, FOM_READ);
		size_t size = file->size();
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

	void write(const void* data, size_t size)
	{
		_bw.write(data, size);
	}

	template <typename T>
	void write(const T& data)
	{
		_bw.write(data);
	}

	const char* platform() const
	{
		return _platform;
	}

private:

	Filesystem& _fs;
	BinaryWriter _bw;
	const char* _platform;
};

} // namespace crown
