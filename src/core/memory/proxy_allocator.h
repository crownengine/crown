/*
 * Copyright (c) 2012-2016 Daniele Bartolini and individual contributors.
 * License: https://github.com/taylor001/crown/blob/master/LICENSE
 */

#pragma once

#include "allocator.h"

namespace crown
{

/// Offers the facility to tag allocators by a string identifier.
/// Proxy allocator is appended to a global linked list when instantiated
/// so that it is possible to later visit that list for debugging purposes.
///
/// @ingroup Memory
class ProxyAllocator : public Allocator
{
	Allocator& _allocator;
	const char* _name;

public:

	/// Tag all allocations made with @a allocator by the given @a name
	ProxyAllocator(Allocator& allocator, const char* name);

	/// @copydoc Allocator::allocate()
	void* allocate(uint32_t size, uint32_t align = Allocator::DEFAULT_ALIGN);

	/// @copydoc Allocator::deallocate()
	void deallocate(void* data);

	/// @copydoc Allocator::allocated_size()
	uint32_t allocated_size(const void* /*ptr*/) { return SIZE_NOT_TRACKED; }

	/// @copydoc Allocator::total_allocated()
	uint32_t total_allocated() { return SIZE_NOT_TRACKED; }

	/// Returns the name of the proxy allocator
	const char* name() const;
};

} // namespace crown
