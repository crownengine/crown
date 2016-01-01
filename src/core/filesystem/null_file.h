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

	/// @copydoc File::File()
	NullFile() {}

	/// @copydoc File::~File()
	virtual ~NullFile() {}

	/// @copydoc File::open()
	void open(const char* /*path*/, FileOpenMode::Enum /*mode*/) {}

	/// @copydoc File::close()
	void close() {}

	/// @copydoc File::seek()
	void seek(uint32_t position) { (void)position; }

	/// @copydoc File::seek_to_end()
	void seek_to_end() {}

	/// @copydoc File::skip()
	void skip(uint32_t bytes) { (void)bytes; }

	/// @copydoc File::read()
	/// @note
	///	Fills buffer with zeroes
	uint32_t read(void* data, uint32_t size)
	{
		for (uint32_t i = 0; i < size; ++i)
		{
			((uint8_t*)data)[i] = 0;
		}
		return size;
	}

	/// @copydoc File::write()
	uint32_t write(const void* /*data*/, uint32_t size)
	{
		return size;
	}

	/// @copydoc File::flush()
	void flush() {};

	/// @copydoc File::is_valid()
	/// @note
	///	Returns always true
	bool is_valid() { return true; }

	/// @copydoc File::end_of_file()
	/// @note
	///	Returns always false
	bool end_of_file() { return false; }

	/// @copydoc File::size()
	/// @note
	///	Returns always 0xFFFFFFFF
	uint32_t size() { return ~0; }

	/// @copydoc File::position()
	/// @note
	///	Returns always zero
	uint32_t position() { return 0; }
};

} // namespace crown
