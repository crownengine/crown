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

#include <android/asset_manager_jni.h>

#include "Assert.h"
#include "OS.h"
#include "OsFile.h"
#include "AndroidOS.h"

namespace crown
{

static AAssetManager*	g_android_asset_manager = NULL;

//-----------------------------------------------------------------------------
OsFile::OsFile(const char* path, FileOpenMode mode)
{
	// Android assets are always read-only
	(void) mode;
	m_mode = FOM_READ;
	m_asset = AAssetManager_open(get_android_asset_manager(), path, AASSET_MODE_RANDOM);

	CE_ASSERT(m_asset != NULL, "Unable to open file: %s", path);
}

//-----------------------------------------------------------------------------
OsFile::~OsFile()
{
	close();
}

//-----------------------------------------------------------------------------
void OsFile::close()
{
	if (m_asset != NULL)
	{
		AAsset_close(m_asset);
		m_asset = NULL;
	}
}

//-----------------------------------------------------------------------------
bool OsFile::is_open() const
{
	return m_asset != NULL;
}

//-----------------------------------------------------------------------------
FileOpenMode OsFile::mode() const
{
	return m_mode;
}

//-----------------------------------------------------------------------------
size_t OsFile::size() const
{
	return AAsset_getLength(m_asset);
}

//-----------------------------------------------------------------------------
size_t OsFile::read(void* data, size_t size)
{
	CE_ASSERT(data != NULL, "Data must be != NULL");

	return (size_t)AAsset_read(m_asset, data, size);
}

//-----------------------------------------------------------------------------
size_t OsFile::write(const void* data, size_t size)
{
	CE_ASSERT(data != NULL, "Data must be != NULL");

	os::printf("Android asset directory is read-only!");

	return 0;
}

//-----------------------------------------------------------------------------
void OsFile::seek(size_t position)
{
	off_t seek_result = AAsset_seek(m_asset, (off_t)position, SEEK_SET);
	CE_ASSERT(seek_result != (off_t) -1, "Failed to seek");
}

//-----------------------------------------------------------------------------
void OsFile::seek_to_end()
{
	off_t seek_result = AAsset_seek(m_asset, 0, SEEK_END);
	CE_ASSERT(seek_result != (off_t) -1, "Failed to seek");
}

//-----------------------------------------------------------------------------
void OsFile::skip(size_t bytes)
{
	off_t seek_result = AAsset_seek(m_asset, (off_t) bytes, SEEK_CUR);
	CE_ASSERT(seek_result != (off_t) -1, "Failed to seek");
}

//-----------------------------------------------------------------------------
size_t OsFile::position() const
{
	return (size_t) (AAsset_getLength(m_asset) - AAsset_getRemainingLength(m_asset));
}

//-----------------------------------------------------------------------------
bool OsFile::eof() const
{
	return AAsset_getRemainingLength(m_asset) == 0;
}

//-----------------------------------------------------------------------------
AAssetManager* get_android_asset_manager()
{
	return g_android_asset_manager;
}

//-----------------------------------------------------------------------------
extern "C" JNIEXPORT void JNICALL Java_crown_android_CrownLib_initAssetManager(JNIEnv* env, jobject obj, jobject assetManager)
{
	g_android_asset_manager = AAssetManager_fromJava(env, assetManager);
}

} // namespace crown

