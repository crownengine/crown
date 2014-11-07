/*
 * Copyright (c) 2012-2014 Daniele Bartolini and individual contributors.
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
public:

	/// Tag all allocations made with @a allocator by the given @a name
	ProxyAllocator(const char* name, Allocator& allocator);

	/// @copydoc Allocator::allocate()
	void* allocate(size_t size, size_t align = Allocator::DEFAULT_ALIGN);

	/// @copydoc Allocator::deallocate()
	void deallocate(void* data);

	/// @copydoc Allocator::allocated_size()
	size_t allocated_size();

	/// Returns the name of the proxy allocator
	const char* name() const;

private:

	Allocator& _allocator;

	const char* _name;
	size_t _total_allocated;
};

} // namespace crown
