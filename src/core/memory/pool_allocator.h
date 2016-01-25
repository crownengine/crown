/*
 * Copyright (c) 2012-2016 Daniele Bartolini and individual contributors.
 * License: https://github.com/taylor001/crown/blob/master/LICENSE
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
	Allocator&	_backing;

	void* _start;
	void* _freelist;
	uint32_t _block_size;
	uint32_t _block_align;

	uint32_t _num_allocations;
	uint32_t _allocated_size;

public:

	/// Uses @a backing to allocate the memory pool for containing exactly
	/// @a num_blocks blocks of @a block_size size each aligned to @a block_align.
	PoolAllocator(Allocator& backing, uint32_t num_blocks, uint32_t block_size, uint32_t block_align = Allocator::DEFAULT_ALIGN);
	~PoolAllocator();

	/// Allocates a block of memory from the memory pool.
	/// @note
	/// The @a size and @a align must match those passed to PoolAllocator::PoolAllocator()
	void* allocate(uint32_t size, uint32_t align = Allocator::DEFAULT_ALIGN);

	/// @copydoc Allocator::deallocate()
	void deallocate(void* data);

	/// @copydoc Allocator::allocated_size()
	uint32_t allocated_size(const void* /*ptr*/) { return SIZE_NOT_TRACKED; }

	/// @copydoc Allocator::total_allocated()
	uint32_t total_allocated();
};

} // namespace crown
