/*
 * Copyright (c) 2012-2021 Daniele Bartolini et al.
 * License: https://github.com/dbartolini/crown/blob/master/LICENSE
 */

#pragma once

#include "core/memory/allocator.h"

namespace crown
{
/// Allocates memory linearly in a stack-like fashion from a
/// predefined chunk. All deallocations must occur in LIFO
/// order.
///
/// @ingroup Memory
struct StackAllocator : public Allocator
{
	struct Header
	{
		u32 offset;
		u32 alloc_id;
	};

	char* _begin;
	char* _top;
	u32 _total_size;
	u32 _allocation_count;

	StackAllocator(char* begin, u32 size);
	~StackAllocator();

	/// @copydoc Allocator::allocate()
	void* allocate(u32 size, u32 align = Allocator::DEFAULT_ALIGN);

	/// @copydoc Allocator::deallocate()
	/// @note
	/// Deallocations must occur in LIFO order i.e. the
	/// last allocation must be freed for first.
	void deallocate(void* data);

	/// @copydoc Allocator::allocated_size()
	u32 allocated_size(const void* /*ptr*/) { return SIZE_NOT_TRACKED; }

	/// @copydoc Allocator::total_allocated()
	u32 total_allocated();
};

} // namespace crown
