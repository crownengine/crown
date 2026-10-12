/*
 * Copyright (c) 2012-2026 Daniele Bartolini et al.
 * SPDX-License-Identifier: MIT
 */

#pragma once

#include "core/containers/array.inl"
#include "core/filesystem/file.h"
#include "core/filesystem/reader_writer.h"
#include "core/math/types.h"
#include "core/strings/string_id.h"
#include <type_traits>

namespace crown
{
inline BinaryWriter::BinaryWriter(File &file)
	: _file(file)
{
}

inline void BinaryWriter::align(const u32 align)
{
	const u32 mask = align - 1;
	const u32 pos = (_file.position() + mask) & ~mask;
	const u32 pad = pos - _file.position();
	const char val = 0;
	for (u32 ii = 0; ii < pad; ++ii)
		_file.write(&val, 1);
}

inline void BinaryWriter::write(const void *data, u32 size)
{
	_file.write(data, size);
}

template<typename T>
inline void BinaryWriter::write(const T &data)
{
	static_assert(std::is_arithmetic<T>::value, "BinaryWriter::write requires an arithmetic type");
	align(alignof(T));
	_file.write(&data, sizeof(T));
}

template<typename T>
inline void BinaryWriter::write(const Array<T> &data)
{
	static_assert(std::is_arithmetic<T>::value, "BinaryWriter::write requires arithmetic array elements");
	if (array::empty(data))
		return;

	const u64 bytes = u64(array::size(data)) * sizeof(T);
	CE_ASSERT(bytes <= UINT32_MAX, "Array is too large to write");
	align(alignof(T));
	write(array::begin(data), u32(bytes));
}

inline void BinaryWriter::write(const StringId32 &data)
{
	write(data._id);
}

inline void BinaryWriter::write(const StringId64 &data)
{
	write(data._id);
}

inline void BinaryWriter::write(const Vector2 &data)
{
	write(data.x);
	write(data.y);
}

inline void BinaryWriter::write(const Vector3 &data)
{
	write(data.x);
	write(data.y);
	write(data.z);
}

inline void BinaryWriter::write(const Vector4 &data)
{
	write(data.x);
	write(data.y);
	write(data.z);
	write(data.w);
}

inline void BinaryWriter::write(const Quaternion &data)
{
	write(data.x);
	write(data.y);
	write(data.z);
	write(data.w);
}

inline void BinaryWriter::write(const Matrix4x4 &data)
{
	write(data.x);
	write(data.y);
	write(data.z);
	write(data.t);
}

inline void BinaryWriter::write(const OBB &data)
{
	write(data.tm);
	write(data.half_extents);
}

inline void BinaryWriter::write(const Sphere &data)
{
	write(data.c);
	write(data.r);
}

template<typename T>
inline void BinaryWriter::write_unaligned(const T &data)
{
	static_assert(std::is_arithmetic<T>::value, "BinaryWriter::write_unaligned requires an arithmetic type");
	_file.write(&data, sizeof(T));
}

inline void BinaryWriter::skip(u32 bytes)
{
	_file.skip(bytes);
}

inline BinaryReader::BinaryReader(File &file)
	: _file(file)
{
}

inline void BinaryReader::align(const u32 align)
{
	const u32 mask = align - 1;
	const u32 pos = (_file.position() + mask) & ~mask;
	const u32 pad = pos - _file.position();
	_file.skip(pad);
}

inline void BinaryReader::read(void *data, u32 size)
{
	_file.read(data, size);
}

template<typename T>
inline void BinaryReader::read(T &data)
{
	align(alignof(T));
	_file.read(&data, sizeof(T));
}

template<typename T>
inline void BinaryReader::read_unaligned(T &data)
{
	_file.read(&data, sizeof(T));
}

inline void BinaryReader::skip(u32 bytes)
{
	_file.skip(bytes);
}

} // namespace crown
