/*
 * Copyright (c) 2012-2016 Daniele Bartolini and individual contributors.
 * License: https://github.com/taylor001/crown/blob/master/LICENSE
 */

#pragma once

#include "types.h"
#include "file.h"

namespace crown
{
/// A writer that offers a convenient way to write to a File
///
/// @ingroup Filesystem
class BinaryWriter
{
	File& _file;

public:

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
class BinaryReader
{
	File& _file;

public:

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
