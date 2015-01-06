/*
 * Copyright (c) 2012-2015 Daniele Bartolini and individual contributors.
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
	NullFile(FileOpenMode mode) : File(mode) {}

	/// @copydoc File::~File()
	virtual ~NullFile() {}

	/// @copydoc File::seek()
	void seek(size_t position) { (void)position; }

	/// @copydoc File::seek_to_end()
	void seek_to_end() {}

	/// @copydoc File::skip()
	void skip(size_t bytes) { (void)bytes; }

	/// @copydoc File::read()
	/// @note
	///	Fills buffer with zeroes
	void read(void* buffer, size_t size)
	{
		for (size_t i = 0; i < size; i++)
		{
			((uint8_t*)buffer)[i] = 0;
		}
	}

	/// @copydoc File::write()
	void write(const void* buffer, size_t size) { (void)buffer; (void)size; }

	/// @copydoc File::copy_to()
	/// @note
	///	Returns always true
	bool copy_to(File& file, size_t size = 0)
	{
		char zero = 0;
		file.write(&zero, size);
		return true;
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
	size_t size() { return ~0; }

	/// @copydoc File::position()
	/// @note
	///	Returns always zero
	size_t position() { return 0; }

	/// @copydoc File::can_read()
	/// @note
	///	Returns always true
	bool can_read() { return true; }

	/// @copydoc File::can_write()
	/// @note
	///	Returns always true
	bool can_write() { return true; }

	/// @copydoc File::can_seek()
	/// @note
	///	Returns always true
	bool can_seek() { return true; }
};

} // namespace crown
