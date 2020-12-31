/*
 * Copyright (c) 2012-2021 Daniele Bartolini et al.
 * License: https://github.com/dbartolini/crown/blob/master/LICENSE
 */

#pragma once

#include "core/filesystem/types.h"
#include "core/types.h"

namespace crown
{
/// An abstraction to access data files.
///
/// @ingroup Filesystem
struct File
{
	///
	File() {}

	///
	virtual ~File() {}

	/// Opens the file at @a path with specified @a mode
	virtual void open(const char* path, FileOpenMode::Enum mode) = 0;

	/// Closes the file.
	virtual void close() = 0;

	/// Returns whether the file is open.
	virtual bool is_open() = 0;

	/// Returns the size of file in bytes.
	virtual u32 size() = 0;

	/// Returns the number of bytes from the beginning of the file
	/// to the cursor position.
	virtual u32 position() = 0;

	/// Returns whether the cursor position is at end of file.
	virtual bool end_of_file() = 0;

	/// Sets the cursor to @a position.
	virtual void seek(u32 position) = 0;

	/// Sets the cursor position to the end of the file.
	virtual void seek_to_end() = 0;

	/// Sets the cursor position to @a bytes after current position.
	virtual void skip(u32 bytes) = 0;

	/// Reads @a size bytes from file.
	virtual u32 read(void* data, u32 size) = 0;

	/// Writes @a size bytes to file.
	virtual u32 write(const void* data, u32 size) = 0;

	/// Forces the previouses write operations to complete.
	virtual void flush() = 0;
};

} // namespace crown
