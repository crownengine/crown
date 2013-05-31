/*
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

#pragma once

#include "File.h"

namespace crown
{


/// Bit bucket file.
/// Discards all data written to it and provides null data reading from it; plain and simple.
class NullFile: public File
{
public:

	/// @a copydoc File::File()
				NullFile(FileOpenMode mode) : File(mode) {}

	/// @a copydoc File::~File()
	virtual		~NullFile() {}

	/// @a copydoc File::seek()
	void		seek(size_t position) { (void)position; }

	/// @a copydoc File::seek_to_end()
	void		seek_to_end() {}

	/// @a copydoc File::skip()
	void		skip(size_t bytes) { (void)bytes; }
				
	/// @a copydoc File::read()
	/// @note
	///	Fills buffer with zeroes
	void		read(void* buffer, size_t size)
	{
		for (size_t i = 0; i < size; i++)
		{
			((uint8_t*)buffer)[i] = 0;
		}
	}

	/// @a copydoc File::write()
	void		write(const void* buffer, size_t size) { (void)buffer; (void)size; }

	/// @a copydoc File::copy_to()
	/// @note
	///	Returns always true
	bool		copy_to(File& file, size_t size = 0)
	{
		char zero = 0;
		file.write(&zero, size);		
		return true;
	}

	/// @a copydoc File::flush()
	void		flush() {};
				
	/// @a copydoc File::is_valid()
	/// @note
	///	Returns always true
	bool		is_valid() { return true; }
				
	/// @a copydoc File::end_of_file()
	/// @note
	///	Returns always false
	bool		end_of_file() { return false; }
				
	/// @a copydoc File::size()
	/// @note
	///	Returns always 0xFFFFFFFF
	size_t		size() { return ~0; }
				
	/// @a copydoc File::position()
	/// @note
	///	Returns always zero
	size_t		position() { return 0; }
				
	/// @a copydoc File::can_read()
	/// @note
	///	Returns always true
	bool		can_read() { return true; }
				
	/// @a copydoc File::can_write()
	/// @note
	///	Returns always true
	bool		can_write() { return true; }
				
	/// @a copydoc File::can_seek()
	/// @note
	///	Returns always true
	bool		can_seek() { return true; }
};

} // namespace crown

