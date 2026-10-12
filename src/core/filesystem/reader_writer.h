/*
 * Copyright (c) 2012-2026 Daniele Bartolini et al.
 * SPDX-License-Identifier: MIT
 */

#pragma once

#include "core/filesystem/types.h"
#include "core/types.h"

namespace crown
{
template<typename T>
struct Array;
struct StringId32;
struct StringId64;
struct Vector2;
struct Vector3;
struct Vector4;
struct Quaternion;
struct Matrix4x4;
struct OBB;
struct Sphere;

/// A writer that offers a convenient way to write to a File
///
/// @ingroup Filesystem
struct BinaryWriter
{
	File &_file;

	///
	explicit BinaryWriter(File &file);

	///
	void align(const u32 align);

	///
	void write(const void *data, u32 size);

	/// Writes an arithmetic value.
	template<typename T>
	void write(const T &data);

	/// Writes an array of arithmetic values.
	template<typename T>
	void write(const Array<T> &data);

	///
	void write(const StringId32 &data);

	///
	void write(const StringId64 &data);

	///
	void write(const Vector2 &data);

	///
	void write(const Vector3 &data);

	///
	void write(const Vector4 &data);

	///
	void write(const Quaternion &data);

	///
	void write(const Matrix4x4 &data);

	///
	void write(const OBB &data);

	///
	void write(const Sphere &data);

	/// Writes an arithmetic value without alignment.
	template<typename T>
	void write_unaligned(const T &data);

	///
	void skip(u32 bytes);
};

/// A reader that offers a convenient way to read from a File
///
/// @ingroup Filesystem
struct BinaryReader
{
	File &_file;

	///
	explicit BinaryReader(File &file);

	///
	void align(const u32 align);

	///
	void read(void *data, u32 size);

	///
	template<typename T>
	void read(T &data);

	///
	template<typename T>
	void read_unaligned(T &data);

	///
	void skip(u32 bytes);
};

} // namespace crown
