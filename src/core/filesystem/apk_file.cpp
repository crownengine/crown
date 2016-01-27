/*
 * Copyright (c) 2012-2016 Daniele Bartolini and individual contributors.
 * License: https://github.com/taylor001/crown/blob/master/LICENSE
 */

#include "config.h"

#if CROWN_PLATFORM_ANDROID

#include "apk_file.h"
#include "error.h"
#include "macros.h"
#include <stdio.h> // SEEK_SET, ...
#include <android/asset_manager.h>

namespace crown
{
ApkFile::ApkFile(AAssetManager* asset_manager)
	: _asset_manager(asset_manager)
	, _asset(NULL)
{
}

ApkFile::~ApkFile()
{
	close();
}

void ApkFile::open(const char* path, FileOpenMode::Enum /*mode*/)
{
	_asset = AAssetManager_open(_asset_manager, path, AASSET_MODE_RANDOM);
	CE_ASSERT(_asset != NULL, "AAssetManager_open: failed to open %s", path);
}

void ApkFile::close()
{
	if (_asset)
	{
		AAsset_close(_asset);
		_asset = NULL;
	}
}

u32 ApkFile::size()
{
	return AAsset_getLength(_asset);
}

u32 ApkFile::position()
{
	return u32(AAsset_getLength(_asset) - AAsset_getRemainingLength(_asset));
}

bool ApkFile::end_of_file()
{
	return AAsset_getRemainingLength(_asset) == 0;
}

void ApkFile::seek(u32 position)
{
	off_t seek_result = AAsset_seek(_asset, (off_t)position, SEEK_SET);
	CE_ASSERT(seek_result != (off_t)-1, "AAsset_seek: error");
	CE_UNUSED(seek_result);
}

void ApkFile::seek_to_end()
{
	off_t seek_result = AAsset_seek(_asset, 0, SEEK_END);
	CE_ASSERT(seek_result != (off_t)-1, "AAsset_seek: error");
	CE_UNUSED(seek_result);
}

void ApkFile::skip(u32 bytes)
{
	off_t seek_result = AAsset_seek(_asset, (off_t)bytes, SEEK_CUR);
	CE_ASSERT(seek_result != (off_t)-1, "AAsset_seek: error");
	CE_UNUSED(seek_result);
}

u32 ApkFile::read(void* data, u32 size)
{
	CE_ASSERT_NOT_NULL(data);
	return (u32)AAsset_read(_asset, data, size);
}

u32 ApkFile::write(const void* /*data*/, u32 /*size*/)
{
	CE_ASSERT(false, "Apk files are read only!");
	return 0;
}

void ApkFile::flush()
{
	// Not needed
}

} // namespace crown

#endif // CROWN_PLATFORM_ANDROID
