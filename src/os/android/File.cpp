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

#include <cassert>
#include <stdio.h>

#include "OS.h"
#include "File.h"
#include "AndroidOS.h"

namespace crown
{

//-----------------------------------------------------------------------------
File::File() :
	m_file_handle(NULL),
	m_mode(SOM_READ)
{
}

//-----------------------------------------------------------------------------
File::~File()
{
	close();
}

//-----------------------------------------------------------------------------
void File::close()
{
	if (m_asset != NULL)
	{
		AAsset_close(m_asset);
		m_asset = NULL;
	}
}

//-----------------------------------------------------------------------------
bool File::is_open() const
{
	return m_asset != NULL;
}

//-----------------------------------------------------------------------------
StreamOpenMode File::mode()
{
	return m_mode;
}

//-----------------------------------------------------------------------------
size_t File::size() const
{
	assert(m_asset != NULL);
	
	return AAsset_getLength(m_asset);
}

//-----------------------------------------------------------------------------
bool File::open(const char* path, StreamOpenMode mode)
{
	assert(!is_open());

	// Android assets are always read-only
	(void) mode;
	m_mode = SOM_READ;

	m_asset = AAssetManager_open(os::get_android_asset_manager(), path, AASSET_MODE_RANDOM);

	if (m_asset == NULL)
	{
		os::printf("Could not open asset %s", path);

		return false;
	}

	return true;
}

//-----------------------------------------------------------------------------
size_t File::read(void* data, size_t size)
{
	assert(m_asset != NULL);
	assert(data != NULL);

	return (size_t)AAsset_read(m_asset, data, size);
}

//-----------------------------------------------------------------------------
size_t File::write(const void* data, size_t size)
{
	assert(m_asset != NULL);
	assert(data != NULL);

	os::printf("Android asset directory is read-only!");

	return 0;
}

//-----------------------------------------------------------------------------
void File::seek(size_t position)
{
	assert(m_asset != NULL);

	assert(AAsset_seek(m_asset, (off_t)position, SEEK_SET) != (off_t) -1);
}

//-----------------------------------------------------------------------------
void File::seek_to_end()
{
	assert(m_asset != NULL);

	assert(AAsset_seek(m_asset, 0, SEEK_END) != (off_t) -1);
}

//-----------------------------------------------------------------------------
void File::skip(size_t bytes)
{
	assert(m_asset != NULL);

	assert(AAsset_seek(m_asset, (off_t) bytes, SEEK_CUR) != (off_t) -1);
}

//-----------------------------------------------------------------------------
size_t File::position() const
{
	assert(m_asset != NULL);
	
	return (size_t) (AAsset_getLength(m_asset) - AAsset_getRemainingLength(m_asset));
}

//-----------------------------------------------------------------------------
bool File::eof() const
{
	assert(m_asset != NULL);
	
	return AAsset_getRemainingLength(m_asset) == 0;
}

} // namespace crown

