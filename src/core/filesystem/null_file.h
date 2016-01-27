/*
 * Copyright (c) 2012-2016 Daniele Bartolini and individual contributors.
 * License: https://github.com/taylor001/crown/blob/master/LICENSE
 */

#pragma once

#include "file.h"

namespace crown
{
/// Bit bucket file.
/// Discards all data written to it and provides null data reading from it; plain and simple.
///
/// @ingroup Filesystem
class NullFile: public File
{
public:

	/// @copydoc File::open()
	void open(const char* /*path*/, FileOpenMode::Enum /*mode*/) {}

	/// @copydoc File::close()
	void close() {}

	/// @copydoc File::size()
	/// @note
	///	Returns always 0xFFFFFFFF
	u32 size() { return ~0; }

	/// @copydoc File::position()
	/// @note
	///	Returns always zero
	u32 position() { return 0; }

	/// @copydoc File::end_of_file()
	/// @note
	///	Returns always false
	bool end_of_file() { return false; }

	/// @copydoc File::seek()
	void seek(u32 position) { (void)position; }

	/// @copydoc File::seek_to_end()
	void seek_to_end() {}

	/// @copydoc File::skip()
	void skip(u32 bytes) { (void)bytes; }

	/// @copydoc File::read()
	/// @note
	///	Fills buffer with zeroes
	u32 read(void* data, u32 size)
	{
		for (u32 i = 0; i < size; ++i)
		{
			((u8*)data)[i] = 0;
		}
		return size;
	}

	/// @copydoc File::write()
	u32 write(const void* /*data*/, u32 size)
	{
		return size;
	}

	/// @copydoc File::flush()
	void flush() {};
};

} // namespace crown
