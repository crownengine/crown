/*
 * Copyright (c) 2012-2014 Daniele Bartolini and individual contributors.
 * License: https://github.com/taylor001/crown/blob/master/LICENSE
 */

#pragma once

#include "allocator.h"

namespace crown
{

/// Allocates memory linearly from a predefined chunk
/// and frees all the allocations with a single call to clear()
///
/// @ingroup Memory
class LinearAllocator : public Allocator
{
public:

	LinearAllocator(Allocator& backing, size_t size);
	LinearAllocator(void* start, size_t size);
	~LinearAllocator();

	/// @copydoc Allocator::allocate()
	void* allocate(size_t size, size_t align = Allocator::DEFAULT_ALIGN);

	/// @copydoc Allocator::deallocate()
	/// @note
	/// The linear allocator does not support deallocating
	/// individual allocations, rather you have to call
	/// clear() to free all allocated memory at once.
	void deallocate(void* data);

	/// Frees all the allocations made by allocate()
	void clear();

	/// @copydoc Allocator::allocated_size()
	size_t allocated_size();

private:

	Allocator* _backing;

	void* _physical_start;
	size_t _total_size;
	size_t _offset;
};

} // namespace crown
