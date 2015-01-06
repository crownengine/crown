/*
 * Copyright (c) 2012-2015 Daniele Bartolini and individual contributors.
 * License: https://github.com/taylor001/crown/blob/master/LICENSE
 */

#include "config.h"

#if CROWN_PLATFORM_ANDROID

#include "apk_file.h"
#include "assert.h"
#include "macros.h"
#include <android/asset_manager.h>

namespace crown
{

ApkFile::ApkFile(AAssetManager* asset_manager, const char* path)
	: File(FOM_READ)
	, _asset(NULL)
{
	_asset = AAssetManager_open(asset_manager, path, AASSET_MODE_RANDOM);
	CE_ASSERT(_asset != NULL, "AAssetManager_open: failed to open %s", path);
}

ApkFile::~ApkFile()
{
	if (_asset != NULL)
	{
		AAsset_close(_asset);
		_asset = NULL;
	}
}

void ApkFile::seek(size_t position)
{
	off_t seek_result = AAsset_seek(_asset, (off_t)position, SEEK_SET);
	CE_ASSERT(seek_result != (off_t) -1, "AAsset_seek: error");
	CE_UNUSED(seek_result);
}

void ApkFile::seek_to_end()
{
	off_t seek_result = AAsset_seek(_asset, 0, SEEK_END);
	CE_ASSERT(seek_result != (off_t) -1, "AAsset_seek: error");
	CE_UNUSED(seek_result);
}

void ApkFile::skip(size_t bytes)
{
	off_t seek_result = AAsset_seek(_asset, (off_t) bytes, SEEK_CUR);
	CE_ASSERT(seek_result != (off_t) -1, "AAsset_seek: error");
	CE_UNUSED(seek_result);
}

void ApkFile::read(void* buffer, size_t size)
{
	CE_ASSERT_NOT_NULL(buffer);
	size_t bytes_read = (size_t) AAsset_read(_asset, buffer, size);
	CE_ASSERT(bytes_read == size, "AAsset_read: requested: %lu, read: %lu", size, bytes_read);
	CE_UNUSED(bytes_read);
}

void ApkFile::write(const void* /*buffer*/, size_t /*size*/)
{
	CE_ASSERT(false, "Apk files are read only!");
}

bool ApkFile::copy_to(File& /*file*/, size_t /*size = 0*/)
{
	CE_ASSERT(false, "Not implemented");
	return false;
}

void ApkFile::flush()
{
	// Not needed
}

bool ApkFile::is_valid()
{
	return _asset != NULL;
}

bool ApkFile::end_of_file()
{
	return AAsset_getRemainingLength(_asset) == 0;
}

size_t ApkFile::size()
{
	return AAsset_getLength(_asset);
}

size_t ApkFile::position()
{
	return (size_t) (AAsset_getLength(_asset) - AAsset_getRemainingLength(_asset));
}

bool ApkFile::can_read() const
{
	return true;
}

bool ApkFile::can_write() const
{
	return false;
}

bool ApkFile::can_seek() const
{
	return true;
}

} // namespace crown

#endif // CROWN_PLATFORM_ANDROID
