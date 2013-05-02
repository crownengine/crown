/*
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

#include "File.h"
#include "Log.h"
#include "MathUtils.h"
#include <cassert>

namespace crown
{

//-----------------------------------------------------------------------------
File::File() :
	m_file_handle(NULL), m_mode(FOM_READ)
{
}

//-----------------------------------------------------------------------------
File::~File()
{
	if (m_file_handle != NULL)
	{
		fclose(m_file_handle);
	}
}

//-----------------------------------------------------------------------------
bool File::is_valid()
{
	return m_file_handle != 0;
}

//-----------------------------------------------------------------------------
FileOpenMode File::mode()
{
	return m_mode;
}

//-----------------------------------------------------------------------------
FILE* File::get_handle()
{
	return m_file_handle;
}

//-----------------------------------------------------------------------------
File* File::open(const char* path, FileOpenMode mode)
{
	File* f = new File();
	f->m_file_handle = fopen(path, 

	/*
		TestFlag(mode, FOM_READ) ?
			(TestFlag(mode, FOM_WRITE) ?
				(TestFlag(mode, FOM_CREATENEW) ? "wb+" : "rb+") : "rb") : (TestFlag(mode, FOM_WRITE) ? "wb" : "rb"));
	*/

	math::test_bitmask(mode, FOM_READ) ?
		(math::test_bitmask(mode, FOM_WRITE) ? "rb+" : "rb") : (math::test_bitmask(mode, FOM_WRITE) ? "wb" : "rb")); 

	if (f->m_file_handle == NULL)
	{
		Log::e("File::Open: Could not open file %s", path);
		return NULL;
	}

	f->m_mode = mode;

	return f;
}

//-----------------------------------------------------------------------------
size_t File::read(void* ptr, size_t size, size_t nmemb)
{
	assert(m_file_handle != NULL);
	assert(ptr != NULL);

	return fread(ptr, size, nmemb, m_file_handle);
}

//-----------------------------------------------------------------------------
size_t File::write(const void* ptr, size_t size, size_t nmemb)
{
	assert(m_file_handle != NULL);
	assert(ptr != NULL);

	return fwrite(ptr, size, nmemb, m_file_handle);
}

//-----------------------------------------------------------------------------
int File::seek(int32_t offset, int whence)
{
	assert(m_file_handle != NULL);

	return fseek(m_file_handle, offset, whence);
}

//-----------------------------------------------------------------------------
int32_t File::tell()
{
	assert(m_file_handle != NULL);
	
	return ftell(m_file_handle);
}

//-----------------------------------------------------------------------------
int File::eof()
{
	assert(m_file_handle != NULL);
	
	return feof(m_file_handle);
}

//-----------------------------------------------------------------------------
size_t File::size()
{
	size_t pos = ftell(m_file_handle);
	fseek(m_file_handle, 0, SEEK_END);
	size_t size = ftell(m_file_handle);
	fseek(m_file_handle, pos, SEEK_SET);

	return size;
}

} // namespace crown

