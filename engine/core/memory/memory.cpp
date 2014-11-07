/*
 * Copyright (c) 2012-2014 Daniele Bartolini and individual contributors.
 * License: https://github.com/taylor001/crown/blob/master/LICENSE
 */

#include "memory.h"
#include "allocator.h"
#include "mutex.h"

// void* operator new(size_t) throw (std::bad_alloc)
// {
// 	CE_ASSERT(false, "operator new forbidden");

// 	return NULL;
// }

// void* operator new[](size_t) throw (std::bad_alloc)
// {
// 	CE_ASSERT(false, "operator new[] forbidden");

// 	return NULL;
// }

// void operator delete(void*) throw ()
// {
// 	CE_ASSERT(false, "operator delete forbidden");
// }

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
			: _allocated_size(0)
			, _allocation_count(0)
		{
		}

		~HeapAllocator()
		{
			CE_ASSERT(_allocation_count == 0 && allocated_size() == 0,
				"Missing %d deallocations causing a leak of %ld bytes", _allocation_count, allocated_size());
		}

		/// @copydoc Allocator::allocate()
		void* allocate(size_t size, size_t align = Allocator::DEFAULT_ALIGN)
		{
			ScopedMutex sm(_mutex);

			size_t actual_size = actual_allocation_size(size, align);

			Header* h = (Header*)malloc(actual_size);
			h->size = actual_size;

			void* data = memory::align_top(h + 1, align);

			pad(h, data);

			_allocated_size += actual_size;
			_allocation_count++;

			return data;
		}

		/// @copydoc Allocator::deallocate()
		void deallocate(void* data)
		{
			ScopedMutex sm(_mutex);

			if (!data)
				return;

			Header* h = header(data);

			_allocated_size -= h->size;
			_allocation_count--;

			free(h);
		}

		/// @copydoc Allocator::allocated_size()
		size_t allocated_size()
		{
			ScopedMutex sm(_mutex);
			return _allocated_size;
		}

		/// Returns the size in bytes of the block of memory pointed by @a data
		size_t get_size(void* data)
		{
			ScopedMutex sm(_mutex);
			Header* h = header(data);
			return h->size;
		}

	private:

		// Holds the number of bytes of an allocation
		struct Header
		{
			uint32_t size;
		};

		size_t actual_allocation_size(size_t size, size_t align)
		{
			return size + align + sizeof(Header);
		}

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

		void* data(Header* header, size_t align)
		{
			return memory::align_top(header + 1, align);
		}

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

		Mutex _mutex;
		size_t _allocated_size;
		uint32_t _allocation_count;
	};
} // namespace memory

namespace memory_globals
{
	using namespace memory;
	// Create default allocators
	char _buffer[1024];
	HeapAllocator* _default_allocator = NULL;

	void init()
	{
		_default_allocator = new (_buffer) HeapAllocator();
	}

	void shutdown()
	{
		_default_allocator->~HeapAllocator();
	}
} // namespace memory_globals

Allocator& default_allocator()
{
	return *memory_globals::_default_allocator;
}

} // namespace crown
