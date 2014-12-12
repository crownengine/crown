/*
 * Copyright (c) 2012-2014 Daniele Bartolini and individual contributors.
 * License: https://github.com/taylor001/crown/blob/master/LICENSE
 */

#pragma once

#include "types.h"
#include "ceassert.h"

namespace crown
{

/// Block of raw memory.
///
/// @ingroup Containers
struct Blob
{
	uint32_t _size;
	uintptr_t _data;
};

/// Functions to manipulate Blob.
///
/// @ingroup Containers
namespace blob
{
	/// Returns the size of the blob @a b.
	uint32_t size(const Blob& b);

	/// Retuns a pointer to the byte at the given absolute @a offset into the blob @a b.
	template <typename T> const T* get(const Blob& b, uint32_t offset);

	/// @copydoc blob::get<T>(const Blob&, uint32_t)
	template <typename T> T* get(Blob& b, uint32_t offset);
} // namespace blob

namespace blob
{
	inline uint32_t size(const Blob& b)
	{
		return b._size;
	}

	template <typename T>
	inline const T* get(const Blob& b, uint32_t offset)
	{
		CE_ASSERT(offset < b._size, "Overflow (size = %d, offset = %d", b._size, offset);
		return (T*) b._data + offset;
	}

	template <typename T>
	inline T* get(Blob& b, uint32_t offset)
	{
		CE_ASSERT(offset < b._size, "Overflow (size = %d, offset = %d", b._size, offset);
		return (T*) b._data + offset;
	}
} // namespace blob
} // namespace crown
