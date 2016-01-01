/*
 * Copyright (c) 2012-2016 Daniele Bartolini and individual contributors.
 * License: https://github.com/taylor001/crown/blob/master/LICENSE
 */

#include "memory.h"
#include "allocator.h"
#include "mutex.h"
#include <stdlib.h> // malloc

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
	// Header stored at the beginning of a memory allocation to indicate the
	// size of the allocated data.
	struct Header {
		uint32_t size;
	};

	// If we need to align the memory allocation we pad the header with this
	// value after storing the size. That way we can
	const uint32_t HEADER_PAD_VALUE = 0xffffffffu;

	// Given a pointer to the header, returns a pointer to the data that follows it.
	inline void *data_pointer(Header *header, uint32_t align) {
		void *p = header + 1;
		return memory::align_top(p, align);
	}

	// Given a pointer to the data, returns a pointer to the header before it.
	inline Header *header(const void *data)
	{
		uint32_t *p = (uint32_t *)data;
		while (p[-1] == HEADER_PAD_VALUE)
			--p;
		return (Header *)p - 1;
	}

	// Stores the size in the header and pads with HEADER_PAD_VALUE up to the
	// data pointer.
	inline void fill(Header *header, void *data, uint32_t size)
	{
		header->size = size;
		uint32_t *p = (uint32_t *)(header + 1);
		while (p < data)
			*p++ = HEADER_PAD_VALUE;
	}
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
			CE_ASSERT(_allocation_count == 0 && total_allocated() == 0
				, "Missing %d deallocations causing a leak of %d bytes"
				, _allocation_count
				, total_allocated()
				);
		}

		/// @copydoc Allocator::allocate()
		void* allocate(uint32_t size, uint32_t align = Allocator::DEFAULT_ALIGN)
		{
			ScopedMutex sm(_mutex);

			uint32_t actual_size = actual_allocation_size(size, align);

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
		uint32_t allocated_size(const void* ptr)
		{
			return get_size(ptr);
		}

		/// @copydoc Allocator::total_allocated()
		uint32_t total_allocated()
		{
			ScopedMutex sm(_mutex);
			return _allocated_size;
		}

		/// Returns the size in bytes of the block of memory pointed by @a data
		uint32_t get_size(const void* data)
		{
			ScopedMutex sm(_mutex);
			Header* h = header(data);
			return h->size;
		}

	private:

		uint32_t actual_allocation_size(uint32_t size, uint32_t align)
		{
			return size + align + sizeof(Header);
		}

		void pad(Header* header, void* data)
		{
			uint32_t* p = (uint32_t*)(header + 1);

			while (p != data)
			{
				*p = HEADER_PAD_VALUE;
				p++;
			}
		}

	private:

		Mutex _mutex;
		uint32_t _allocated_size;
		uint32_t _allocation_count;
	};

	// Copyright (C) 2012 Bitsquid AB
	// License: https://bitbucket.org/bitsquid/foundation/src/default/LICENCSE
	//
	// An allocator used to allocate temporary "scratch" memory. The allocator
	// uses a fixed size ring buffer to services the requests.
	//
	// Memory is always always allocated linearly. An allocation pointer is
	// advanced through the buffer as memory is allocated and wraps around at
	// the end of the buffer. Similarly, a free pointer is advanced as memory
	// is freed.
	//
	// It is important that the scratch allocator is only used for short-lived
	// memory allocations. A long lived allocator will lock the "free" pointer
	// and prevent the "allocate" pointer from proceeding past it, which means
	// the ring buffer can't be used.
	//
	// If the ring buffer is exhausted, the scratch allocator will use its backing
	// allocator to allocate memory instead.
	class ScratchAllocator : public Allocator
	{
		Allocator &_backing;

		// Start and end of the ring buffer.
		char *_begin, *_end;

		// Pointers to where to allocate memory and where to free memory.
		char *_allocate, *_free;

	public:
		/// Creates a ScratchAllocator. The allocator will use the backing
		/// allocator to create the ring buffer and to service any requests
		/// that don't fit in the ring buffer.
		///
		/// size specifies the size of the ring buffer.
		ScratchAllocator(Allocator &backing, uint32_t size) : _backing(backing) {
			_begin = (char *)_backing.allocate(size);
			_end = _begin + size;
			_allocate = _begin;
			_free = _begin;
		}

		~ScratchAllocator() {
			CE_ASSERT(_free == _allocate, "Memory leak");
			_backing.deallocate(_begin);
		}

		bool in_use(void *p)
		{
			if (_free == _allocate)
				return false;
			if (_allocate > _free)
				return p >= _free && p < _allocate;
			return p >= _free || p < _allocate;
		}

		virtual void *allocate(uint32_t size, uint32_t align) {
			CE_ASSERT(align % 4 == 0, "Must be 4-byte aligned");
			size = ((size + 3)/4)*4;

			char *p = _allocate;
			Header *h = (Header *)p;
			char *data = (char *)data_pointer(h, align);
			p = data + size;

			// Reached the end of the buffer, wrap around to the beginning.
			if (p > _end) {
				h->size = (_end - (char *)h) | 0x80000000u;

				p = _begin;
				h = (Header *)p;
				data = (char *)data_pointer(h, align);
				p = data + size;
			}

			// If the buffer is exhausted use the backing allocator instead.
			if (in_use(p))
				return _backing.allocate(size, align);

			fill(h, data, p - (char *)h);
			_allocate = p;
			return data;
		}

		virtual void deallocate(void *p) {
			if (!p)
				return;

			if (p < _begin || p >= _end) {
				_backing.deallocate(p);
				return;
			}

			// Mark this slot as free
			Header *h = header(p);
			CE_ASSERT((h->size & 0x80000000u) == 0, "Not free");
			h->size = h->size | 0x80000000u;

			// Advance the free pointer past all free slots.
			while (_free != _allocate) {
				Header *h = (Header *)_free;
				if ((h->size & 0x80000000u) == 0)
					break;

				_free += h->size & 0x7fffffffu;
				if (_free == _end)
					_free = _begin;
			}
		}

		virtual uint32_t allocated_size(const void *p) {
			Header *h = header(p);
			return h->size - ((char *)p - (char *)h);
		}

		virtual uint32_t total_allocated() {
			return _end - _begin;
		}
	};
} // namespace memory

namespace memory_globals
{
	using namespace memory;

	static const uint32_t SIZE = sizeof(HeapAllocator)
		+ sizeof(ScratchAllocator)
		;
	char _buffer[SIZE];
	HeapAllocator* _default_allocator = NULL;
	ScratchAllocator* _default_scratch_allocator = NULL;

	void init()
	{
		_default_allocator = new (_buffer) HeapAllocator();
		_default_scratch_allocator = new (_buffer + sizeof(HeapAllocator)) ScratchAllocator(*_default_allocator, 1024*1024);
	}

	void shutdown()
	{
		_default_scratch_allocator->~ScratchAllocator();
		_default_allocator->~HeapAllocator();
	}
} // namespace memory_globals

Allocator& default_allocator()
{
	return *memory_globals::_default_allocator;
}

Allocator& default_scratch_allocator()
{
	return *memory_globals::_default_scratch_allocator;
}

} // namespace crown
