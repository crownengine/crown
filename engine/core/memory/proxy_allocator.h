/*
 * Copyright (c) 2012-2014 Daniele Bartolini and individual contributors.
 * License: https://github.com/taylor001/crown/blob/master/LICENSE
 */

#pragma once

#include "allocator.h"
#include "macros.h"

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

public:

	/// Returns the total number of proxy allocators.
	/// in the global list.
	static uint32_t count();

	/// Returns the proxy allocator @name or NULL if not found.
	static ProxyAllocator* find(const char* name);

	/// Returns the first proxy allocator in the global list or
	/// NULL if the list is empty.
	static ProxyAllocator* begin();

	/// Returns the next proxy allocator to @a a in the global list
	/// or NULL if end-of-list is reached.
	static ProxyAllocator* next(ProxyAllocator* a);

private:

	Allocator& _allocator;
	
	const char* _name;
	size_t _total_allocated;
	ProxyAllocator* _next;
};

} // namespace crown
