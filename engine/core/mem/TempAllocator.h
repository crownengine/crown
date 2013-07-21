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

namespace crown
{

/// Allocates from a fixed-size buffer.
/// When the internal memory is exhausted or the allocation size exceeds
/// the remainig internal storage, the backing allocator is used instead.
/// The memory is automatically freed when the allocator is destroyed.
template <size_t SIZE>
class TempAllocator : public Allocator
{
public:

	/// Uses the @a backing allocator when internal memory is exahusted
	/// or the allocation size exceeds the remaining storage.
				TempAllocator(Allocator& backing = default_allocator());
				~TempAllocator();

	/// @copydoc Allocator::allocate()
	void*		allocate(size_t size, size_t align = memory::DEFAULT_ALIGN);

	/// Does nothing, the memory is automatically freed when the
	/// allocator is destroyed.
	void		deallocate(void* data);

	/// @copydoc Allocator::allocated_size()
	size_t		allocated_size();

private:

	Allocator&	m_backing;

	size_t		m_allocated_size;
	uint8_t		m_buffer[SIZE];
};

typedef TempAllocator<256> TempAllocator256;
typedef TempAllocator<512> TempAllocator512;
typedef TempAllocator<1024> TempAllocator1024;
typedef TempAllocator<2048> TempAllocator2048;
typedef TempAllocator<4096> TempAllocator4096;

//-----------------------------------------------------------------------------
template <size_t SIZE>
inline TempAllocator<SIZE>::TempAllocator(Allocator& backing) :
	m_backing(backing),
	m_allocated_size(0)
{
}

//-----------------------------------------------------------------------------
template <size_t SIZE>
inline TempAllocator<SIZE>::~TempAllocator()
{
}

//-----------------------------------------------------------------------------
template <size_t SIZE>
inline void* TempAllocator<SIZE>::allocate(size_t size, size_t align)
{
	size_t actual_size = size + align;

	if (actual_size > (SIZE - m_allocated_size))
	{
		return m_backing.allocate(size, align);
	}

	void* user_ptr = memory::align_top((void*) &m_buffer[m_allocated_size], align);

	m_allocated_size += actual_size;

	return user_ptr;
}

//-----------------------------------------------------------------------------
template <size_t SIZE>
inline void TempAllocator<SIZE>::deallocate(void* data)
{
	(void) data;
}

//-----------------------------------------------------------------------------
template <size_t SIZE>
inline size_t TempAllocator<SIZE>::allocated_size()
{
	return m_allocated_size;
}

} // namespace crown
