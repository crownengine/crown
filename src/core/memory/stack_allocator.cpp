/*
 * Copyright (c) 2012-2015 Daniele Bartolini and individual contributors.
 * License: https://github.com/taylor001/crown/blob/master/LICENSE
 */

#include "stack_allocator.h"
#include "memory.h"

namespace crown
{

StackAllocator::StackAllocator(void* start, uint32_t size)
	: _physical_start(start)
	, _top(start)
	, _total_size(size)
	, _allocation_count(0)
{
}

StackAllocator::~StackAllocator()
{
	CE_ASSERT(_allocation_count == 0 && total_allocated() == 0
		, "Missing %d deallocations causing a leak of %d bytes"
		, _allocation_count
		, total_allocated()
		);
}

void* StackAllocator::allocate(uint32_t size, uint32_t align)
{
	const uint32_t actual_size = sizeof(Header) + size + align;

	// Memory exhausted
	if ((char*) _top + actual_size > (char*) _physical_start + _total_size)
	{
		return NULL;
	}

	// The offset from TOS to the start of the buffer
	uint32_t offset = (char*) _top - (char*) _physical_start;

	// Align user data only, ignore header alignment
	_top = (char*) memory::align_top((char*) _top + sizeof(Header), align) - sizeof(Header);

	Header* header = (Header*) _top;
	header->offset = offset;
	header->alloc_id = _allocation_count;

	void* user_ptr = (char*) _top + sizeof(Header);
	_top = (char*) _top + actual_size;

	_allocation_count++;

	return user_ptr;
}

void StackAllocator::deallocate(void* data)
{
	if (!data)
		return;

	Header* data_header = (Header*) ((char*)data - sizeof(Header));

	CE_ASSERT(data_header->alloc_id == _allocation_count - 1,
		"Deallocations must occur in LIFO order");

	_top = (char*) _physical_start + data_header->offset;

	_allocation_count--;
}

uint32_t StackAllocator::total_allocated()
{
	return (char*) _top - (char*) _physical_start;
}

} // namespace crown
