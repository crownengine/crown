/*
 * Copyright (c) 2012-2015 Daniele Bartolini and individual contributors.
 * License: https://github.com/taylor001/crown/blob/master/LICENSE
 */

#pragma once

#include "allocator.h"
#include "array.h"

namespace crown
{

/// Dynamic block of raw memory.
///
/// @ingroup Containers
typedef Array<char> DynamicBlob;

/// Functions to manipulate DynamicBlob.
///
/// @ingroup Containers
namespace dynamic_blob
{
	/// Returns the size of the blob @a b.
	uint32_t size(const DynamicBlob& b);

	/// Retuns a pointer to the byte at the given absolute @a offset into the blob @a b.
	template <typename T> const T* get(const DynamicBlob& b, uint32_t offset);

	/// @copydoc blob::get<T>(const DynamicBlob&, uint32_t)
	template <typename T> T* get(DynamicBlob& b, uint32_t offset);

	/// Appends @a num @a items to the blob @a b and returns the size of the
	/// blob after the append operation.
	template <typename T> uint32_t push(DynamicBlob& b, const T* items, uint32_t num);

	/// Appends @a blob to @a b and returns the size of the blob after the append operation.
	uint32_t push_blob(DynamicBlob& b, const DynamicBlob& other);
} // namespace dynamic_blob

namespace dynamic_blob
{
	inline uint32_t size(const DynamicBlob& b)
	{
		return array::size(b);
	}

	template <typename T>
	inline const T* get(const DynamicBlob& b, uint32_t offset)
	{
		CE_ASSERT(offset < size(b), "Overflow (size = %d, offset = %d", size(b), offset);
		return (T*) array::begin(b) + offset;
	}

	template <typename T>
	inline T* get(DynamicBlob& b, uint32_t offset)
	{
		CE_ASSERT(offset < size(b), "Overflow (size = %d, offset = %d", size(b), offset);
		return (T*) array::begin(b) + offset;
	}

	template <typename T>
	inline uint32_t push(DynamicBlob& b, const T* items, uint32_t num)
	{
		return array::push(b, (const char*) items, num);
	}

	inline uint32_t push_blob(DynamicBlob& b, const DynamicBlob& other)
	{
		return array::push(b, array::begin(other), size(other));
	}
} // namespace dynamic_blob
} // namespace crown
