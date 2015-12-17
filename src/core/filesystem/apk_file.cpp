/*
 * Copyright (c) 2012-2015 Daniele Bartolini and individual contributors.
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

ApkFile::ApkFile(AAssetManager* asset_manager, const char* path)
	: File(FileOpenMode::READ)
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

void ApkFile::seek(uint32_t position)
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

void ApkFile::skip(uint32_t bytes)
{
	off_t seek_result = AAsset_seek(_asset, (off_t)bytes, SEEK_CUR);
	CE_ASSERT(seek_result != (off_t)-1, "AAsset_seek: error");
	CE_UNUSED(seek_result);
}

uint32_t ApkFile::read(void* data, uint32_t size)
{
	CE_ASSERT_NOT_NULL(data);
	return (uint32_t)AAsset_read(_asset, data, size);
}

uint32_t ApkFile::write(const void* /*data*/, uint32_t /*size*/)
{
	CE_ASSERT(false, "Apk files are read only!");
	return 0;
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

uint32_t ApkFile::size()
{
	return AAsset_getLength(_asset);
}

uint32_t ApkFile::position()
{
	return (uint32_t)(AAsset_getLength(_asset) - AAsset_getRemainingLength(_asset));
}

} // namespace crown

#endif // CROWN_PLATFORM_ANDROID
