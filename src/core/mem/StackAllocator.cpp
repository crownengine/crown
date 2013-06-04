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

#include "StackAllocator.h"
#include "OS.h"

namespace crown
{

//-----------------------------------------------------------------------------
StackAllocator::StackAllocator(void* start, size_t size) :
	m_physical_start(start),
	m_total_size(size),
	m_top(start),
	m_allocation_count(0)
{
}

//-----------------------------------------------------------------------------
StackAllocator::~StackAllocator()
{
	CE_ASSERT(m_allocation_count == 0 && allocated_size() == 0,
		"Missing %d deallocations causing a leak of %d bytes", m_allocation_count, allocated_size());
}

//-----------------------------------------------------------------------------
void* StackAllocator::allocate(size_t size, size_t align)
{
	const size_t actual_size = sizeof(Header) + size + align;

	// Memory exhausted
	if ((char*) m_top + actual_size > (char*) m_physical_start + m_total_size)
	{
		return NULL;
	}

	// The offset from TOS to the start of the buffer
	uint32_t offset = (char*) m_top - (char*) m_physical_start;

	// Align user data only, ignore header alignment
	m_top = (char*) memory::align_top((char*) m_top + sizeof(Header), align) - sizeof(Header);

	Header* header = (Header*) m_top;
	header->offset = offset;
	header->alloc_id = m_allocation_count;

	void* user_ptr = (char*) m_top + sizeof(Header);
	m_top = (char*) m_top + actual_size;

	m_allocation_count++;

	return user_ptr;
}

//-----------------------------------------------------------------------------
void StackAllocator::deallocate(void* data)
{
	Header* data_header = (Header*) ((char*)data - sizeof(Header));

	CE_ASSERT(data_header->alloc_id == m_allocation_count - 1,
		"Deallocations must occur in LIFO order");

	m_top = (char*) m_physical_start + data_header->offset;

	m_allocation_count--;
}

//-----------------------------------------------------------------------------
size_t StackAllocator::allocated_size()
{
	return (char*) m_top - (char*) m_physical_start;
}

//-----------------------------------------------------------------------------
void StackAllocator::dump()
{
	uint8_t* mem = (uint8_t*)m_physical_start;
	for (size_t i = 0; i < allocated_size(); i++)
	{
		if ((i % sizeof(void*)) == 0)
		{
			os::printf("\n");
			os::printf("%.2X = ", (size_t)(&mem[i]) & 0xFF);
		}
		os::printf("%.2X ", mem[i]);
	}

	os::printf("\n");
}

} // namespace crown
