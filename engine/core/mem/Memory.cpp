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

#include "Memory.h"
#include "Allocator.h"
#include "Mutex.h"

// //-----------------------------------------------------------------------------
// void* operator new(size_t) throw (std::bad_alloc)
// {
// 	CE_ASSERT(false, "operator new forbidden");

// 	return NULL;
// }

// //-----------------------------------------------------------------------------
// void* operator new[](size_t) throw (std::bad_alloc)
// {
// 	CE_ASSERT(false, "operator new[] forbidden");

// 	return NULL;
// }

// //-----------------------------------------------------------------------------
// void operator delete(void*) throw ()
// {
// 	CE_ASSERT(false, "operator delete forbidden");
// }

// //-----------------------------------------------------------------------------
// void operator delete[](void*) throw ()
// {
// 	CE_ASSERT(false, "operator delete[] forbidden");
// }

namespace crown
{
namespace memory
{

/// Allocator based on C malloc().
class HeapAllocator : public Allocator
{
public:

	HeapAllocator()
		: m_allocated_size(0)
		, m_allocation_count(0)
	{
	}

	~HeapAllocator()
	{
		CE_ASSERT(m_allocation_count == 0 && allocated_size() == 0,
			"Missing %d deallocations causing a leak of %ld bytes", m_allocation_count, allocated_size());
	}

	/// @copydoc Allocator::allocate()
	void* allocate(size_t size, size_t align = Allocator::DEFAULT_ALIGN)
	{
		ScopedMutex sm(m_mutex);

		size_t actual_size = actual_allocation_size(size, align);

		Header* h = (Header*)malloc(actual_size);
		h->size = actual_size;

		void* data = memory::align_top(h + 1, align);

		pad(h, data);

		m_allocated_size += actual_size;
		m_allocation_count++;

		return data;
	}

	/// @copydoc Allocator::deallocate()
	void deallocate(void* data)
	{
		ScopedMutex sm(m_mutex);

		if (!data)
			return;

		Header* h = header(data);

		m_allocated_size -= h->size;
		m_allocation_count--;

		free(h);
	}

	/// @copydoc Allocator::allocated_size()
	size_t allocated_size()
	{
		ScopedMutex sm(m_mutex);
		return m_allocated_size;
	}

	/// Returns the size in bytes of the block of memory pointed by @a data
	size_t get_size(void* data)
	{
		ScopedMutex sm(m_mutex);
		Header* h = header(data);
		return h->size;
	}

private:

	// Holds the number of bytes of an allocation
	struct Header
	{
		uint32_t	size;
	};

	//-----------------------------------------------------------------------------
	size_t actual_allocation_size(size_t size, size_t align)
	{
		return size + align + sizeof(Header);
	}

	//-----------------------------------------------------------------------------
	Header* header(void* data)
	{
		uint32_t* ptr = (uint32_t*)data;
		ptr--;

		while (*ptr == memory::PADDING_VALUE)
		{
			ptr--;
		}

		return (Header*)ptr;
	}

	//-----------------------------------------------------------------------------
	void* data(Header* header, size_t align)
	{
		return memory::align_top(header + 1, align);
	}

	//-----------------------------------------------------------------------------
	void pad(Header* header, void* data)
	{
		uint32_t* p = (uint32_t*)(header + 1);

		while (p != data)
		{
			*p = memory::PADDING_VALUE;
			p++;
		}
	}

private:

	Mutex		m_mutex;
	size_t		m_allocated_size;
	uint32_t	m_allocation_count;
};

// Create default allocators
static char buffer[1024];
static HeapAllocator* g_default_allocator = NULL;

void init()
{
	g_default_allocator = new (buffer) HeapAllocator();
}

void shutdown()
{
	g_default_allocator->~HeapAllocator();
}

} // namespace memory

Allocator& default_allocator()
{
	return *memory::g_default_allocator;
}

} // namespace crown
