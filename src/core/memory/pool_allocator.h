/*
 * Copyright (c) 2012-2026 Daniele Bartolini et al.
 * SPDX-License-Identifier: MIT
 */

#pragma once

#include "core/memory/allocator.h"

namespace crown
{
/// Allocates objects from a growing memory pool.
///
/// @ingroup Memory
struct PoolAllocator : public Allocator
{
	Allocator &_backing;

	struct PoolHeader *_start;
	void *_freelist;
	u32 _num_blocks;
	u32 _block_size;
	u32 _block_align;

	/// Uses @a backing to allocate a memory pool for objects that fit into @a block_size bytes. The
	/// pool consists of a chunk of @a num_blocks initially, and grows adding new chunks when
	/// needed.
	PoolAllocator(Allocator &backing, u32 num_blocks, u32 block_size, u32 block_align = Allocator::DEFAULT_ALIGN);

	///
	~PoolAllocator();

	/// Allocates a block of memory from the memory pool.
	/// @note
	/// @a size and @a align must be smaller-or-equal-to those passed to PoolAllocator::PoolAllocator().
	void *allocate(u32 size, u32 align = Allocator::DEFAULT_ALIGN) override;

	/// @copydoc Allocator::deallocate()
	void deallocate(void *data) override;

	/// @copydoc Allocator::allocated_size()
	u32 allocated_size(const void *ptr) override
	{
		CE_UNUSED(ptr);
		return SIZE_NOT_TRACKED;
	}

	/// @copydoc Allocator::total_allocated()
	u32 total_allocated() override
	{
		return SIZE_NOT_TRACKED;
	}
};

} // namespace crown
