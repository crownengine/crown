/*
 * Copyright (c) 2012-2021 Daniele Bartolini et al.
 * License: https://github.com/dbartolini/crown/blob/master/LICENSE
 */

#include "core/memory/allocator.h"
#include "core/memory/memory.inl"
#include "core/memory/stack_allocator.h"

namespace crown
{
StackAllocator::StackAllocator(char* begin, u32 size)
	: _begin(begin)
	, _top(begin)
	, _total_size(size)
	, _allocation_count(0)
{
}

StackAllocator::~StackAllocator()
{
	CE_ASSERT(_allocation_count == 0 && total_allocated() == 0
		, "Missing %u deallocations causing a leak of %u bytes"
		, _allocation_count
		, total_allocated()
		);
}

void* StackAllocator::allocate(u32 size, u32 align)
{
	const u32 actual_size = sizeof(Header) + size + align;

	// Memory exhausted
	if (_top + actual_size > _begin + _total_size)
	{
		return NULL;
	}

	// The offset from TOS to the start of the buffer
	u32 offset = u32(_top - _begin);

	// Align user data only, ignore header alignment
	_top = (char*)memory::align_top(_top + sizeof(Header), align) - sizeof(Header);

	Header* header = (Header*) _top;
	header->offset = offset;
	header->alloc_id = _allocation_count;

	void* user_ptr = _top + sizeof(Header);
	_top = _top + actual_size;

	_allocation_count++;

	return user_ptr;
}

void StackAllocator::deallocate(void* data)
{
	if (!data)
		return;

	Header* data_header = (Header*)((char*)data - sizeof(Header));

	CE_ASSERT(data_header->alloc_id == _allocation_count - 1
		, "Deallocations must occur in LIFO order"
		);

	_top = _begin + data_header->offset;

	_allocation_count--;
}

u32 StackAllocator::total_allocated()
{
	return u32(_top - _begin);
}

} // namespace crown
