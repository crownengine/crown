/*
 * Copyright (c) 2012-2014 Daniele Bartolini and individual contributors.
 * License: https://github.com/taylor001/crown/blob/master/LICENSE
 */

#pragma once

#include "file.h"
#include <sys/types.h>
#include <android/asset_manager.h>

namespace crown
{

class ApkFile : public File
{
public:

	/// Opens the given @a filename.
	ApkFile(AAssetManager* asset_manager, const char* filename);
	~ApkFile();

	/// @copydoc File::seek()
	void seek(size_t position);

	/// @copydoc File::seek_to_end()
	void seek_to_end();

	/// @copydoc File::skip()
	void skip(size_t bytes);

	/// @copydoc File::read()
	void read(void* buffer, size_t size);

	/// @copydoc File::write()
	void write(const void* buffer, size_t size);

	/// @copydoc File::copy_to()
	bool copy_to(File& file, size_t size = 0);

	/// @copydoc File::flush()
	void flush();

	/// @copydoc File::is_valid()
	bool is_valid();

	/// @copydoc File::end_of_file()
	bool end_of_file();

	/// @copydoc File::size()
	size_t size();

	/// @copydoc File::position()
	size_t position();

	/// @copydoc File::can_read()
	bool can_read() const;

	/// @copydoc File::can_write()
	bool can_write() const;

	/// @copydoc File::can_seek()
	bool can_seek() const;

private:

	AAsset* _asset;
};

} // namespace crown
