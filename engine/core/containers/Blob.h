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

#include "Types.h"
#include "Assert.h"

namespace crown
{

/// Block of raw memory.
///
/// @ingroup Containers
struct Blob
{
	uint32_t m_size;
	uintptr_t m_data;
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
		return b.m_size;
	}

	template <typename T>
	inline const T* get(const Blob& b, uint32_t offset)
	{
		CE_ASSERT(offset < b.m_size, "Overflow (size = %d, offset = %d", b.m_size, offset);
		return (T*) b.m_data + offset;
	}

	template <typename T>
	inline T* get(Blob& b, uint32_t offset)
	{
		CE_ASSERT(offset < b.m_size, "Overflow (size = %d, offset = %d", b.m_size, offset);
		return (T*) b.m_data + offset;
	}
} // namespace blob
} // namespace crown
