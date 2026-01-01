/*
 * Copyright (c) 2012-2026 Daniele Bartolini et al.
 * SPDX-License-Identifier: MIT
 */

#include "core/error/error.inl"
#include "core/memory/proxy_allocator.h"
#include "core/profiler.h"

namespace crown
{
ProxyAllocator::ProxyAllocator(Allocator &allocator, const char *name)
	: _allocator(allocator)
	, _name(name)
{
	CE_ASSERT(name != NULL, "Name must be != NULL");
}

void *ProxyAllocator::allocate(u32 size, u32 align)
{
	void *p = _allocator.allocate(size, align);
	ALLOCATE_MEMORY(_name, _allocator.allocated_size(p));
	return p;
}

void ProxyAllocator::deallocate(void *data)
{
	DEALLOCATE_MEMORY(_name, (data == NULL) ? 0 :_allocator.allocated_size((void *)data));
	_allocator.deallocate(data);
}

void *ProxyAllocator::reallocate(void *data, u32 size, u32 align)
{
	if (!data) {
		void *ptr = _allocator.reallocate(data, size, align);
		ALLOCATE_MEMORY(_name, _allocator.allocated_size(ptr));
		return ptr;
	}

	if (size == 0) {
		DEALLOCATE_MEMORY(_name, _allocator.allocated_size(data));
		return _allocator.reallocate(data, size, align);
	}

	const u32 data_size = _allocator.allocated_size(data);
	void *ptr = _allocator.reallocate(data, size, align);
	ALLOCATE_MEMORY(_name, _allocator.allocated_size(ptr));
	DEALLOCATE_MEMORY(_name, data_size);
	return ptr;
}

const char *ProxyAllocator::name() const
{
	return _name;
}

} // namespace crown
