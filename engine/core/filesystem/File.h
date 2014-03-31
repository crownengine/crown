/*
Copyright (c) 2013 Daniele Bartolini, Michele Rossi
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

#include "Types.h"

namespace crown
{

enum FileOpenMode
{
	FOM_READ		= 1,
	FOM_WRITE		= 2
};

class Compressor;

/// An abstraction to access data files.
/// 
/// It represents a flow of data attached to a 'file' which can be an archived file,
/// a regular file, a location in memory or anything that can be read or wrote.
/// A File is an abstraction to interact with these in an uniform way; every file
/// comes with a convenient set of methods to facilitate reading from it, writing to
/// it and so on.
///
/// @ingroup Filesystem
class File
{
public:

	/// Opens the file with the given @a mode
						File(FileOpenMode mode) : m_open_mode(mode) {}
	virtual				~File() {};

	/// Sets the position indicator of the file to position.
	virtual void		seek(size_t position) = 0;

	/// Sets the position indicator to the end of the file
	virtual void		seek_to_end() = 0;

	/// Sets the position indicator to bytes after current position
	virtual void		skip(size_t bytes) = 0;

	/// Reads a block of data from the file.
	virtual void		read(void* buffer, size_t size) = 0;

	/// Writes a block of data to the file.
	virtual void		write(const void* buffer, size_t size) = 0;

	/// Copies a chunk of 'size' bytes of data from this to another file.
	virtual bool		copy_to(File& file, size_t size = 0) = 0;

	/// Zips a chunk of 'size' bytes of data from this to another file using compressor.
	virtual bool		compress_to(File& file, size_t size, size_t& compressed_size, Compressor& compressor);

	/// Unzip a zipped file of data from this to another file using compressor.
	virtual bool		uncompress_to(File& file, size_t& uncompressed_size, Compressor& compressor);

	/// Forces the previouses write operations to complete.
	/// Generally, when a File is attached to a file,
	/// write operations are not performed instantly, the output data
	/// may be stored to a temporary buffer before making its way to
	/// the file. This method forces all the pending output operations
	/// to be written to the file.
	virtual void		flush() = 0;

	/// Returns whether the file is valid.
	/// A file is valid when the buffer where it operates
	/// exists. (i.e. a file descriptor is attached to the file, 
	/// a memory area is attached to the file etc.)
	virtual bool		is_valid() = 0;

	/// Returns whether the position is at end of file.
	virtual bool		end_of_file() = 0;

	/// Returns the size of file in bytes.
	virtual size_t		size() = 0;

	/// Returns the current position in file.
	/// Generally, for binary data, it means the number of bytes
	/// from the beginning of the file.
	virtual size_t		position() = 0;

	/// Returns whether the file can be read.
	virtual bool		can_read() const = 0;

	/// Returns whether the file can be wrote.
	virtual bool		can_write() const = 0;

	/// Returns whether the file can be sought.
	virtual bool		can_seek() const = 0;

protected:

	FileOpenMode		m_open_mode;
};

} // namespace crown

