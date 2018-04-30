/*
 * Copyright (c) 2012-2018 Daniele Bartolini and individual contributors.
 * License: https://github.com/dbartolini/crown/blob/master/LICENSE
 */

#pragma once

#include "core/filesystem/file.h"
#include "core/types.h"

namespace crown
{
/// A writer that offers a convenient way to write to a File
///
/// @ingroup Filesystem
struct BinaryWriter
{
	File& _file;

	BinaryWriter(File& file)
		: _file(file)
	{
	}

	void write(const void* data, u32 size)
	{
		_file.write(data, size);
	}

	template <typename T>
	void write(const T& data)
	{
		_file.write(&data, sizeof(T));
	}

	void skip(u32 bytes)
	{
		_file.skip(bytes);
	}
};

/// A reader that offers a convenient way to read from a File
///
/// @ingroup Filesystem
struct BinaryReader
{
	File& _file;

	BinaryReader(File& file)
		: _file(file)
	{
	}

	void read(void* data, u32 size)
	{
		_file.read(data, size);
	}

	template <typename T>
	void read(T& data)
	{
		_file.read(&data, sizeof(T));
	}

	void skip(u32 bytes)
	{
		_file.skip(bytes);
	}
};

} // namespace crown
