/*
 * Copyright (c) 2012-2026 Daniele Bartolini et al.
 * SPDX-License-Identifier: MIT
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
	File()
	{
	}

	///
	virtual ~File()
	{
	}

	/// Opens the file at @a path with specified @a mode
	virtual void open(const char *path, FileOpenMode::Enum mode) = 0;

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

	/// Reads @a size bytes from this into @a data.
	virtual u32 read(void *data, u32 size) = 0;

	/// Writes @a size bytes from @a data to this.
	virtual u32 write(const void *data, u32 size) = 0;

	/// Forces the previous write()s to be transferred to the underlying storage device.
	/// Returns 0 if success, -1 otherwise.
	virtual s32 sync() = 0;
};

namespace file
{
	/// Copies @a input_size bytes from @a input to @a output.
	u32 copy(File &output, File &input, u32 input_size);

} // namespace file

} // namespace crown
