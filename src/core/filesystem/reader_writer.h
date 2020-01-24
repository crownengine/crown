/*
 * Copyright (c) 2012-2020 Daniele Bartolini and individual contributors.
 * License: https://github.com/dbartolini/crown/blob/master/LICENSE
 */

#pragma once

#include "core/filesystem/types.h"
#include "core/types.h"

namespace crown
{
/// A writer that offers a convenient way to write to a File
///
/// @ingroup Filesystem
struct BinaryWriter
{
	File& _file;

	///
	BinaryWriter(File& file);

	///
	void write(const void* data, u32 size);

	///
	template <typename T>
	void write(const T& data);

	///
	void skip(u32 bytes);
};

/// A reader that offers a convenient way to read from a File
///
/// @ingroup Filesystem
struct BinaryReader
{
	File& _file;

	///
	BinaryReader(File& file);

	///
	void read(void* data, u32 size);

	///
	template <typename T>
	void read(T& data);

	///
	void skip(u32 bytes);
};

} // namespace crown
