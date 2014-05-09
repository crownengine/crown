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

#include "ApkFile.h"
#include "Assert.h"
#include "Macros.h"
#include "Log.h"
#include <android/asset_manager.h>

namespace crown
{

extern AAssetManager* g_android_asset_manager;

//-----------------------------------------------------------------------------
AAssetManager* get_android_asset_manager()
{
	return g_android_asset_manager;
}

//-----------------------------------------------------------------------------
ApkFile::ApkFile(const char* path)
	: File(FOM_READ), m_asset(NULL)
{
	m_asset = AAssetManager_open(get_android_asset_manager(), path, AASSET_MODE_RANDOM);
	CE_ASSERT(m_asset != NULL, "Unable to open file: %s", path);
}

//-----------------------------------------------------------------------------
ApkFile::~ApkFile()
{
	if (m_asset != NULL)
	{
		AAsset_close(m_asset);
		m_asset = NULL;
	}
}

//-----------------------------------------------------------------------------
void ApkFile::seek(size_t position)
{
	off_t seek_result = AAsset_seek(m_asset, (off_t)position, SEEK_SET);
	CE_ASSERT(seek_result != (off_t) -1, "Failed to seek");
	CE_UNUSED(seek_result);
}

//-----------------------------------------------------------------------------
void ApkFile::seek_to_end()
{
	off_t seek_result = AAsset_seek(m_asset, 0, SEEK_END);
	CE_ASSERT(seek_result != (off_t) -1, "Failed to seek to end");
	CE_UNUSED(seek_result);
}

//-----------------------------------------------------------------------------
void ApkFile::skip(size_t bytes)
{
	off_t seek_result = AAsset_seek(m_asset, (off_t) bytes, SEEK_CUR);
	CE_ASSERT(seek_result != (off_t) -1, "Failed to skip");
	CE_UNUSED(seek_result);
}

//-----------------------------------------------------------------------------
void ApkFile::read(void* buffer, size_t size)
{
	CE_ASSERT_NOT_NULL(buffer);

	size_t bytes_read = (size_t) AAsset_read(m_asset, buffer, size);
	CE_ASSERT(bytes_read == size, "Failed to read from file: requested: %lu, read: %lu", size, bytes_read);
	CE_UNUSED(bytes_read);
}

//-----------------------------------------------------------------------------
void ApkFile::write(const void* /*buffer*/, size_t /*size*/)
{
	CE_ASSERT(false, "Attempt to write to android assets folder!");
}

//-----------------------------------------------------------------------------
bool ApkFile::copy_to(File& /*file*/, size_t /*size = 0*/)
{
	CE_ASSERT(false, "Not implemented yet :(");
	return false;
}

//-----------------------------------------------------------------------------
void ApkFile::flush()
{
	// Not needed
}

//-----------------------------------------------------------------------------
bool ApkFile::is_valid()
{
	return m_asset != NULL;
}

//-----------------------------------------------------------------------------
bool ApkFile::end_of_file()
{
	return AAsset_getRemainingLength(m_asset) == 0;
}

//-----------------------------------------------------------------------------
size_t ApkFile::size()
{
	return AAsset_getLength(m_asset);
}

//-----------------------------------------------------------------------------
size_t ApkFile::position()
{
	return (size_t) (AAsset_getLength(m_asset) - AAsset_getRemainingLength(m_asset));
}

//-----------------------------------------------------------------------------
bool ApkFile::can_read() const
{
	return true;
}

//-----------------------------------------------------------------------------
bool ApkFile::can_write() const
{
	return false;
}

//-----------------------------------------------------------------------------
bool ApkFile::can_seek() const
{
	return true;
}

} // namespace crown