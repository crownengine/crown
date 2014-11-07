/*
 * Copyright (c) 2012-2014 Daniele Bartolini and individual contributors.
 * License: https://github.com/taylor001/crown/blob/master/LICENSE
 */

#include "proxy_allocator.h"
#include "assert.h"

namespace crown
{

ProxyAllocator::ProxyAllocator(const char* name, Allocator& allocator)
	: _allocator(allocator)
	, _name(name)
	, _total_allocated(0)
{
	CE_ASSERT(name != NULL, "Name must be != NULL");
}

void* ProxyAllocator::allocate(size_t size, size_t align)
{
	_total_allocated += size;
	return _allocator.allocate(size, align);
}

void ProxyAllocator::deallocate(void* data)
{
	_allocator.deallocate(data);
}

size_t ProxyAllocator::allocated_size()
{
	return _total_allocated;
}

const char* ProxyAllocator::name() const
{
	return _name;
}

} // namespace crown
