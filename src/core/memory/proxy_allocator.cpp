/*
 * Copyright (c) 2012-2016 Daniele Bartolini and individual contributors.
 * License: https://github.com/taylor001/crown/blob/master/LICENSE
 */

#include "proxy_allocator.h"
#include "error.h"
#include "profiler.h"

namespace crown
{
ProxyAllocator::ProxyAllocator(Allocator& allocator, const char* name)
	: _allocator(allocator)
	, _name(name)
{
	CE_ASSERT(name != NULL, "Name must be != NULL");
}

void* ProxyAllocator::allocate(u32 size, u32 align)
{
	void* p = _allocator.allocate(size, align);
	ALLOCATE_MEMORY(_name, _allocator.allocated_size(p));
	return p;
}

void ProxyAllocator::deallocate(void* data)
{
	DEALLOCATE_MEMORY(_name, (data == NULL) ? 0 :_allocator.allocated_size((const void*)data));
	_allocator.deallocate(data);
}

const char* ProxyAllocator::name() const
{
	return _name;
}

} // namespace crown
