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

#include "Filesystem.h"
#include "FileSource.h"
#include "DynamicString.h"

namespace crown
{

//-----------------------------------------------------------------------------
Filesystem::Filesystem(FileSource& source)
	: m_source(&source)
{
}

//-----------------------------------------------------------------------------
Filesystem::~Filesystem()
{
}

//-----------------------------------------------------------------------------
File* Filesystem::open(const char* path, FileOpenMode mode)
{
	return m_source->open(path, mode);
}

//-----------------------------------------------------------------------------
void Filesystem::close(File* file)
{
	m_source->close(file);
}

//-----------------------------------------------------------------------------
void Filesystem::create_directory(const char* path)
{
	m_source->create_directory(path);
}

//-----------------------------------------------------------------------------
void Filesystem::delete_directory(const char* path)
{
	m_source->delete_directory(path);
}

//-----------------------------------------------------------------------------
void Filesystem::create_file(const char* path)
{
	m_source->create_file(path);
}

//-----------------------------------------------------------------------------
void Filesystem::delete_file(const char* path)
{
	m_source->delete_file(path);
}

//-----------------------------------------------------------------------------
void Filesystem::get_absolute_path(const char* path, DynamicString& os_path)
{
	m_source->get_absolute_path(path, os_path);
}

} // namespace crown
