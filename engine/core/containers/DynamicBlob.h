/*
Copyright (c) 2013 Daniele Bartolini, Michele Rossi
Copyright (c) 2012 Daniele Bartolini, Simone Boscaratto

Permission is hereby granted, free of charge, to any person
obtaining a copy of this software and associated documentation
files (the "Software"), to deal in the Software without
restriction, including without limitation the rights to use,
copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the
Software is furnished to do so, subject to the following
conditions:

The above copyright notice and this permission notice shall be
included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
OTHER DEALINGS IN THE SOFTWARE.
*/

#pragma once

#include "Allocator.h"
#include "Array.h"

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
