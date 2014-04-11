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
#include "Memory.h"

namespace crown
{

/// Allocates from a fixed-size buffer.
/// When the internal memory is exhausted or the allocation size exceeds
/// the remainig internal storage, the backing allocator is used instead.
/// The memory is automatically freed when the allocator is destroyed.
///
/// @ingroup Memory
template <size_t SIZE>
class TempAllocator : public Allocator
{
public:

	/// Uses the @a backing allocator when internal memory is exahusted
	/// or the allocation size exceeds the remaining storage.
	TempAllocator(Allocator& backing = default_allocator());
	~TempAllocator();

	/// @copydoc Allocator::allocate()
	void* allocate(size_t size, size_t align = Allocator::DEFAULT_ALIGN);

	/// Does nothing, the memory is automatically freed when the
	/// allocator is destroyed.
	void deallocate(void* data);

	/// @copydoc Allocator::allocated_size()
	size_t allocated_size();

private:

	Allocator&	m_backing;

	char* m_begin;
	char* m_end;
	char* m_cur;
	size_t m_chunk_size;
	char m_buffer[SIZE];
};

typedef TempAllocator<64> TempAllocator64;
typedef TempAllocator<128> TempAllocator128;
typedef TempAllocator<256> TempAllocator256;
typedef TempAllocator<512> TempAllocator512;
typedef TempAllocator<1024> TempAllocator1024;
typedef TempAllocator<2048> TempAllocator2048;
typedef TempAllocator<4096> TempAllocator4096;

//-----------------------------------------------------------------------------
template <size_t SIZE>
inline TempAllocator<SIZE>::TempAllocator(Allocator& backing)
	: m_backing(backing)
	, m_begin(&m_buffer[0])
	, m_end(&m_buffer[SIZE - 1])
	, m_cur(&m_buffer[0])
	, m_chunk_size(4 * 1024)
{
	*(void**) m_begin = 0;
	m_cur += sizeof(void*);
}

//-----------------------------------------------------------------------------
template <size_t SIZE>
inline TempAllocator<SIZE>::~TempAllocator()
{
	union { char* as_char; void** as_dvoid; };
	as_char = m_buffer;

	void *p = *(void **)as_dvoid;
	while (p)
	{
		void *next = *(void **)p;
		m_backing.deallocate(p);
		p = next;
	}
}

//-----------------------------------------------------------------------------
template <size_t SIZE>
inline void* TempAllocator<SIZE>::allocate(size_t size, size_t align)
{
	m_cur = (char*) memory::align_top(m_cur, align);

	if (size > m_end - m_cur)
	{
		uint32_t to_allocate = sizeof(void*) + size + align;

		if (to_allocate < m_chunk_size)
		{
			to_allocate = m_chunk_size;
		}

		m_chunk_size *= 2;

		void *p = m_backing.allocate(to_allocate);
		*(void **)m_begin = p;
		m_cur = m_begin = (char*) p;
		m_end = m_begin + to_allocate;
		*(void**) m_begin = 0;
		m_cur += sizeof(void*);
		memory::align_top(p, align);
	}

	void *result = m_cur;
	m_cur += size;
	return result;
}

//-----------------------------------------------------------------------------
template <size_t SIZE>
inline void TempAllocator<SIZE>::deallocate(void* /*data*/)
{
}

//-----------------------------------------------------------------------------
template <size_t SIZE>
inline size_t TempAllocator<SIZE>::allocated_size()
{
	return 0;
}

} // namespace crown
