/*
 * Copyright (c) 2012-2021 Daniele Bartolini et al.
 * License: https://github.com/dbartolini/crown/blob/master/LICENSE
 */

#include "core/error/error.inl"
#include "core/memory/proxy_allocator.h"
#include "device/profiler.h"

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
