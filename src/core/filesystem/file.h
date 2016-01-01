/*
 * Copyright (c) 2012-2016 Daniele Bartolini and individual contributors.
 * License: https://github.com/taylor001/crown/blob/master/LICENSE
 */

#pragma once

#include "types.h"
#include "filesystem_types.h"

namespace crown
{

/// An abstraction to access data files.
///
/// @ingroup Filesystem
class File
{
public:

	File() {}
	virtual ~File() {}

	/// Opens the file at @a path with specified @a mode
	virtual void open(const char* path, FileOpenMode::Enum mode) = 0;

	/// Closes the file.
	virtual void close() = 0;

	/// Sets the position indicator of the file to position.
	virtual void seek(uint32_t position) = 0;

	/// Sets the position indicator to the end of the file
	virtual void seek_to_end() = 0;

	/// Sets the position indicator to bytes after current position
	virtual void skip(uint32_t bytes) = 0;

	/// Reads a block of data from the file.
	virtual uint32_t read(void* data, uint32_t size) = 0;

	/// Writes a block of data to the file.
	virtual uint32_t write(const void* data, uint32_t size) = 0;

	/// Forces the previouses write operations to complete.
	virtual void flush() = 0;

	/// Returns whether the file is valid.
	/// A file is valid when the buffer where it operates
	/// exists. (i.e. a file descriptor is attached to the file,
	/// a memory area is attached to the file etc.)
	virtual bool is_valid() = 0;

	/// Returns whether the position is at end of file.
	virtual bool end_of_file() = 0;

	/// Returns the size of file in bytes.
	virtual uint32_t size() = 0;

	/// Returns the current position in file.
	/// Generally, for binary data, it means the number of bytes
	/// from the beginning of the file.
	virtual uint32_t position() = 0;
};

} // namespace crown
