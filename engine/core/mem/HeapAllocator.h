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
#include "ScopedMutex.h"

namespace crown
{

/// Allocator based on C malloc().
class HeapAllocator : public Allocator
{
public:

				HeapAllocator();
				~HeapAllocator();

	/// @copydoc Allocator::allocate()
	void*		allocate(size_t size, size_t align = memory::DEFAULT_ALIGN);

	/// @copydoc Allocator::deallocate()
	void		deallocate(void* data);

	/// @copydoc Allocator::allocated_size()
	size_t		allocated_size();

	/// Returns the size in bytes of the block of memory pointed by @a data
	size_t		get_size(void* data);

private:

	// Holds the number of bytes of an allocation
	struct Header
	{
		uint32_t	size;
	};

	size_t		actual_allocation_size(size_t size, size_t align);
	Header*		header(void* data);
	void*		data(Header* header, size_t align);
	void		pad(Header* header, void* data);

private:

	Mutex		m_mutex;
	size_t		m_allocated_size;
	uint32_t	m_allocation_count;
};

} // namespace crown

