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
#include "AndroidOS.h"
#include <cassert>

namespace crown
{

//-----------------------------------------------------------------------------
File::File() :
	m_asset(NULL), m_mode(FOM_READ)
{
}

//-----------------------------------------------------------------------------
File::~File()
{
	if (m_asset != NULL)
	{
		AAsset_close(m_asset);
	}
}

//-----------------------------------------------------------------------------
bool File::is_valid()
{
	return m_asset != NULL;
}

//-----------------------------------------------------------------------------
FileOpenMode File::mode()
{
	return m_mode;
}

//-----------------------------------------------------------------------------
File* File::open(const char* path, FileOpenMode mode)
{
	File* f = new File();

	f->m_asset = AAssetManager_open(os::get_android_asset_manager(), path, AASSET_MODE_RANDOM);

	if (f->m_asset == NULL)
	{
		Log::E("File::Open: Could not open file %s", path);
		return NULL;
	}

	f->m_mode = FOM_READ;

	return f;
}

//-----------------------------------------------------------------------------
size_t File::read(void* ptr, size_t size, size_t nmemb)
{
	assert(m_asset != NULL);
	assert(ptr != NULL);

	return (size_t)AAsset_read(m_asset, ptr, size * nmemb);
}

//-----------------------------------------------------------------------------
size_t File::write(const void* ptr, size_t size, size_t nmemb)
{
	Log::W("Cannot write to Android asset directory!!!");
}

//-----------------------------------------------------------------------------
int File::seek(int32_t offset, int whence)
{
	assert(m_asset != NULL);

	return AAsset_seek(m_asset, (off_t)offset, whence);
}

//-----------------------------------------------------------------------------
int32_t File::tell()
{
	assert(m_asset != NULL);
	
	return (int32_t)(AAsset_getLength(m_asset) - AAsset_getRemainingLength(m_asset));
}

//-----------------------------------------------------------------------------
int File::eof()
{
	assert(m_asset != NULL);
	
	return (int)(AAsset_getRemainingLength(m_asset) == 0);
}

//-----------------------------------------------------------------------------
size_t File::size()
{
	assert(m_asset != NULL);
	
	return AAsset_getLength(m_asset);
}

} // namespace crown

