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

#pragma once

#include "Allocator.h"

namespace crown
{

/// Allocates memory linearly in a stack-like fashion from a
/// predefined chunk. All deallocations must occur in LIFO
/// order.
class StackAllocator : public Allocator
{
public:

				StackAllocator(void* start, size_t size);
				~StackAllocator();

	/// @copydoc Allocator::allocate()
	void*		allocate(size_t size, size_t align = memory::DEFAULT_ALIGN);

	/// @copydoc Allocator::deallocate()
	/// @note
	/// Deallocations must occur in LIFO order i.e. the
	/// last allocation must be freed for first.
	void		deallocate(void* data);

	/// @copydoc Allocator::allocated_size()
	size_t		allocated_size();

	void		dump();

private:

	struct Header
	{
		uint32_t offset;
		uint32_t alloc_id;
	};

	void*		m_physical_start;
	size_t		m_total_size;

	void*		m_top;

	uint32_t	m_allocation_count;
};

} // namespace crown
