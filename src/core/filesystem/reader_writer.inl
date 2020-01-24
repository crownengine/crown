/*
 * Copyright (c) 2012-2020 Daniele Bartolini and individual contributors.
 * License: https://github.com/dbartolini/crown/blob/master/LICENSE
 */

#pragma once

#include "core/filesystem/file.h"
#include "core/filesystem/reader_writer.h"

namespace crown
{
inline BinaryWriter::BinaryWriter(File& file)
	: _file(file)
{
}

inline void BinaryWriter::write(const void* data, u32 size)
{
	_file.write(data, size);
}

template <typename T>
inline void BinaryWriter::write(const T& data)
{
	_file.write(&data, sizeof(T));
}

inline void BinaryWriter::skip(u32 bytes)
{
	_file.skip(bytes);
}

inline BinaryReader::BinaryReader(File& file)
	: _file(file)
{
}

inline void BinaryReader::read(void* data, u32 size)
{
	_file.read(data, size);
}

template <typename T>
inline void BinaryReader::read(T& data)
{
	_file.read(&data, sizeof(T));
}

inline void BinaryReader::skip(u32 bytes)
{
	_file.skip(bytes);
}

} // namespace crown
