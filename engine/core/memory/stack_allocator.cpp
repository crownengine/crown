/*
Copyright (c) 2013 Daniele Bartolini, Michele Rossi
Copyright (c) 2012 Daniele Bartolini, Simone Boscaratto

Permission is hereby granted, free of charge, to any person
obtaining a copy of this software and associated documentation
files (the "Software"), to deal in the Software without
restriction, including without limitation the rights to use,
copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the
Software is furnished to do so, subject to the following
conditions:

The above copyright notice and this permission notice shall be
included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
OTHER DEALINGS IN THE SOFTWARE.
*/

#include "stack_allocator.h"
#include "memory.h"

namespace crown
{

StackAllocator::StackAllocator(void* start, size_t size)
	: _physical_start(start)
	, _total_size(size)
	, _top(start)
	, _allocation_count(0)
{
}

StackAllocator::~StackAllocator()
{
	CE_ASSERT(_allocation_count == 0 && allocated_size() == 0,
		"Missing %d deallocations causing a leak of %ld bytes", _allocation_count, allocated_size());
}

void* StackAllocator::allocate(size_t size, size_t align)
{
	const size_t actual_size = sizeof(Header) + size + align;

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

size_t StackAllocator::allocated_size()
{
	return (char*) _top - (char*) _physical_start;
}

} // namespace crown
