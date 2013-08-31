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

#include "File.h"
#include "Vector.h"
#include "DynamicString.h"

namespace crown
{

/// Abstract base class for providing access to files
/// on a particular physical/logical device.
class FileSource
{
public:

	FileSource(bool can_write) : m_can_write(can_write) {}
	virtual ~FileSource() {}

	virtual File* open(const char* path, FileOpenMode mode) = 0;
	virtual void close(File* file) = 0;

	virtual void create_directory(const char* path) = 0;
	virtual void delete_directory(const char* path) = 0;
	virtual void create_file(const char* path) = 0;
	virtual void delete_file(const char* path) = 0;
	virtual void list_files(const char* path, Vector<DynamicString>& files) = 0;
	virtual void get_absolute_path(const char* path, DynamicString& abs_path) = 0;

	bool can_write() const
	{
		return m_can_write;
	}

protected:

	bool m_can_write;
};

} // namespace crown
