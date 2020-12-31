/*
 * Copyright (c) 2012-2021 Daniele Bartolini et al.
 * License: https://github.com/dbartolini/crown/blob/master/LICENSE
 */

#pragma once

#include "core/memory/allocator.h"

namespace crown
{
/// Allocates fixed-size memory blocks from a fixed memory pool.
/// The backing allocator is used to allocate the memory pool.
///
/// @ingroup Memory
struct PoolAllocator : public Allocator
{
	Allocator&	_backing;

	void* _start;
	void* _freelist;
	u32 _block_size;
	u32 _block_align;

	u32 _num_allocations;
	u32 _allocated_size;

	/// Uses @a backing to allocate the memory pool for containing exactly
	/// @a num_blocks blocks of @a block_size size each aligned to @a block_align.
	PoolAllocator(Allocator& backing, u32 num_blocks, u32 block_size, u32 block_align = Allocator::DEFAULT_ALIGN);
	~PoolAllocator();

	/// Allocates a block of memory from the memory pool.
	/// @note
	/// The @a size and @a align must match those passed to PoolAllocator::PoolAllocator()
	void* allocate(u32 size, u32 align = Allocator::DEFAULT_ALIGN);

	/// @copydoc Allocator::deallocate()
	void deallocate(void* data);

	/// @copydoc Allocator::allocated_size()
	u32 allocated_size(const void* /*ptr*/) { return SIZE_NOT_TRACKED; }

	/// @copydoc Allocator::total_allocated()
	u32 total_allocated();
};

} // namespace crown
