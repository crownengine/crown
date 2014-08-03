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

#include "allocator.h"

namespace crown
{

/// Allocates fixed-size memory blocks from a fixed memory pool.
/// The backing allocator is used to allocate the memory pool.
///
/// @ingroup Memory
class PoolAllocator : public Allocator
{
public:

	/// Uses @a backing to allocate the memory pool for containing exactly
	/// @a num_blocks blocks of @a block_size size each aligned to @a block_align.
	PoolAllocator(Allocator& backing, size_t num_blocks, size_t block_size, size_t block_align = Allocator::DEFAULT_ALIGN);
	~PoolAllocator();

	/// Allocates a block of memory from the memory pool.
	/// @note
	/// The @a size and @a align must match those passed to PoolAllocator::PoolAllocator()
	void* allocate(size_t size, size_t align = Allocator::DEFAULT_ALIGN);

	/// @copydoc Allocator::deallocate()
	void deallocate(void* data);

	/// @copydoc Allocator::allocated_size()
	size_t allocated_size();

private:

	Allocator&	m_backing;

	void* m_start;
	void* m_freelist;
	size_t m_block_size;
	size_t m_block_align;

	uint32_t m_num_allocations;
	size_t m_allocated_size;
};

} // namespace crown
