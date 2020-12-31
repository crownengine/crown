/*
 * Copyright (c) 2012-2021 Daniele Bartolini et al.
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

inline void BinaryWriter::align(const u32 align)
{
	const u32 mask = align-1;
	const u32 pos = (_file.position() + mask) & ~mask;
	const u32 pad = pos - _file.position();
	const char val = 0;
	for (u32 ii = 0; ii < pad; ++ii)
		_file.write(&val, 1);
}

inline void BinaryWriter::write(const void* data, u32 size)
{
	_file.write(data, size);
}

template <typename T>
inline void BinaryWriter::write(const T& data)
{
	align(alignof(T));
	_file.write(&data, sizeof(T));
}

template <typename T>
inline void BinaryWriter::write_unaligned(const T& data)
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

inline void BinaryReader::align(const u32 align)
{
	const u32 mask = align-1;
	const u32 pos = (_file.position() + mask) & ~mask;
	const u32 pad = pos - _file.position();
	_file.skip(pad);
}

inline void BinaryReader::read(void* data, u32 size)
{
	_file.read(data, size);
}

template <typename T>
inline void BinaryReader::read(T& data)
{
	align(alignof(T));
	_file.read(&data, sizeof(T));
}

template <typename T>
inline void BinaryReader::read_unaligned(T& data)
{
	_file.read(&data, sizeof(T));
}

inline void BinaryReader::skip(u32 bytes)
{
	_file.skip(bytes);
}

} // namespace crown
