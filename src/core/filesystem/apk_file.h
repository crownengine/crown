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
	void seek(u32 position);

	/// @copydoc File::seek_to_end()
	void seek_to_end();

	/// @copydoc File::skip()
	void skip(u32 bytes);

	/// @copydoc File::read()
	u32 read(void* data, u32 size);

	/// @copydoc File::write()
	u32 write(const void* data, u32 size);

	/// @copydoc File::flush()
	void flush();

	/// @copydoc File::is_valid()
	bool is_valid();

	/// @copydoc File::end_of_file()
	bool end_of_file();

	/// @copydoc File::size()
	u32 size();

	/// @copydoc File::position()
	u32 position();
};

} // namespace crown

#endif // CROWN_PLATFORM_ANDROID
