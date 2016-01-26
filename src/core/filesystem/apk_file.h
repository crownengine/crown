/*
 * Copyright (c) 2012-2016 Daniele Bartolini and individual contributors.
 * License: https://github.com/taylor001/crown/blob/master/LICENSE
 */

#pragma once

#include "platform.h"

#if CROWN_PLATFORM_ANDROID

#include "file.h"
#include <sys/types.h>
#include <android/asset_manager.h>

namespace crown
{
/// Provides common facilities to access Android APK files.
///
/// @ingroup Filesystem
class ApkFile : public File
{
	AAssetManager* _asset_manager;
	AAsset* _asset;

public:

	ApkFile(AAssetManager* asset_manager);
	~ApkFile();

	/// @copydoc File::open()
	void open(const char* path, FileOpenMode::Enum mode);

	/// @copydoc File::close()
	void close();

	/// @copydoc File::seek()
	void seek(uint32_t position);

	/// @copydoc File::seek_to_end()
	void seek_to_end();

	/// @copydoc File::skip()
	void skip(uint32_t bytes);

	/// @copydoc File::read()
	uint32_t read(void* data, uint32_t size);

	/// @copydoc File::write()
	uint32_t write(const void* data, uint32_t size);

	/// @copydoc File::flush()
	void flush();

	/// @copydoc File::is_valid()
	bool is_valid();

	/// @copydoc File::end_of_file()
	bool end_of_file();

	/// @copydoc File::size()
	uint32_t size();

	/// @copydoc File::position()
	uint32_t position();
};

#endif // CROWN_PLATFORM_ANDROID

} // namespace crown
