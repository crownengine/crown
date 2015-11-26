/*
 * Copyright (c) 2012-2015 Daniele Bartolini and individual contributors.
 * License: https://github.com/taylor001/crown/blob/master/LICENSE
 */

#include "linear_allocator.h"
#include "memory.h"

namespace crown
{

LinearAllocator::LinearAllocator(Allocator& backing, uint32_t size)
	: _backing(&backing)
	, _physical_start(NULL)
	, _total_size(size)
	, _offset(0)
{
	_physical_start = backing.allocate(size);
}

LinearAllocator::LinearAllocator(void* start, uint32_t size)
	: _backing(NULL)
	, _physical_start(start)
	, _total_size(size)
	, _offset(0)
{
}

LinearAllocator::~LinearAllocator()
{
	if (_backing)
		_backing->deallocate(_physical_start);

	CE_ASSERT(_offset == 0, "Memory leak of %d bytes, maybe you forgot to call clear()?", _offset);
}

void* LinearAllocator::allocate(uint32_t size, uint32_t align)
{
	const uint32_t actual_size = size + align;

	// Memory exhausted
	if (_offset + actual_size > _total_size)
		return NULL;

	void* user_ptr = memory::align_top((char*) _physical_start + _offset, align);

	_offset += actual_size;

	return user_ptr;
}

void LinearAllocator::deallocate(void* /*data*/)
{
	// Single deallocations not supported. Use clear().
}

void LinearAllocator::clear()
{
	_offset = 0;
}

} // namespace crown
