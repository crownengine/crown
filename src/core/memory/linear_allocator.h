/*
 * Copyright (c) 2012-2026 Daniele Bartolini et al.
 * SPDX-License-Identifier: MIT
 */

#pragma once

#include "core/memory/allocator.h"

namespace crown
{
/// Allocates memory linearly from a fixed chunk of memory
/// and frees all the allocations with a single call to clear().
///
/// @ingroup Memory
struct LinearAllocator : public Allocator
{
	Allocator *_backing;
	void *_physical_start;
	u32 _total_size;
	u32 _offset;

	/// Allocates @a size bytes from @a backing.
	LinearAllocator(Allocator &backing, u32 size);

	/// Uses @a size bytes of memory from @a start.
	LinearAllocator(void *start, u32 size);

	///
	~LinearAllocator();

	/// @copydoc Allocator::allocate()
	void *allocate(u32 size, u32 align = Allocator::DEFAULT_ALIGN) override;

	/// @copydoc Allocator::deallocate()
	/// @note
	/// The linear allocator does not support deallocating
	/// individual allocations, rather you have to call
	/// clear() to free all allocated memory at once.
	void deallocate(void *data) override;

	/// Frees all the allocations made by allocate()
	void clear();

	/// @copydoc Allocator::allocated_size()
	u32 allocated_size(const void * /*ptr*/) override
	{
		return SIZE_NOT_TRACKED;
	}

	/// @copydoc Allocator::total_allocated()
	u32 total_allocated() override
	{
		return _offset;
	}
};

} // namespace crown
