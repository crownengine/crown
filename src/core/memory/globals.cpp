/*
 * Copyright (c) 2012-2021 Daniele Bartolini et al.
 * License: https://github.com/dbartolini/crown/blob/master/LICENSE
 */

#include "core/error/error.inl"
#include "core/memory/allocator.h"
#include "core/memory/globals.h"
#include "core/memory/memory.inl"
#include "core/thread/scoped_mutex.inl"
#include <stdlib.h> // malloc

// void* operator new(size_t) throw (std::bad_alloc)
// {
// 	CE_FATAL("operator new forbidden");

// 	return NULL;
// }

// void* operator new[](size_t) throw (std::bad_alloc)
// {
// 	CE_FATAL("operator new[] forbidden");

// 	return NULL;
// }

// void operator delete(void*) throw ()
// {
// 	CE_FATAL("operator delete forbidden");
// }

// void operator delete[](void*) throw ()
// {
// 	CE_FATAL("operator delete[] forbidden");
// }

namespace crown
{
namespace memory
{
	// Header stored at the beginning of a memory allocation to indicate the
	// size of the allocated data.
	struct Header
	{
		u32 size;
	};

	// If we need to align the memory allocation we pad the header with this
	// value after storing the size. That way we can
	const u32 HEADER_PAD_VALUE = 0xffffffffu;

	// Given a pointer to the header, returns a pointer to the data that follows it.
	inline void *data_pointer(Header* header, u32 align) {
		void *p = header + 1;
		return memory::align_top(p, align);
	}

	// Given a pointer to the data, returns a pointer to the header before it.
	inline Header*header(const void* data)
	{
		u32 *p = (u32*)data;
		while (p[-1] == HEADER_PAD_VALUE)
			--p;
		return (Header*)p - 1;
	}

	// Stores the size in the header and pads with HEADER_PAD_VALUE up to the
	// data pointer.
	inline void fill(Header*header, void *data, u32 size)
	{
		header->size = size;
		u32 *p = (u32*)(header + 1);
		while (p < data)
			*p++ = HEADER_PAD_VALUE;
	}

	inline u32 actual_allocation_size(u32 size, u32 align)
	{
		return size + align + sizeof(Header);
	}

	inline void pad(Header* header, void* data)
	{
		u32* p = (u32*)(header + 1);

		while (p != data)
		{
			*p = HEADER_PAD_VALUE;
			p++;
		}
	}

	/// Allocator based on C malloc().
	struct HeapAllocator : public Allocator
	{
		Mutex _mutex;
		u32 _allocated_size;
		u32 _allocation_count;

		HeapAllocator()
			: _allocated_size(0)
			, _allocation_count(0)
		{
		}

		~HeapAllocator()
		{
			CE_ASSERT(_allocation_count == 0 && total_allocated() == 0
				, "Missing %u deallocations causing a leak of %u bytes"
				, _allocation_count
				, total_allocated()
				);
		}

		/// @copydoc Allocator::allocate()
		void* allocate(u32 size, u32 align = Allocator::DEFAULT_ALIGN)
		{
			ScopedMutex sm(_mutex);

			u32 actual_size = actual_allocation_size(size, align);

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
		u32 allocated_size(const void* ptr)
		{
			return get_size(ptr);
		}

		/// @copydoc Allocator::total_allocated()
		u32 total_allocated()
		{
			ScopedMutex sm(_mutex);
			return _allocated_size;
		}

		/// Returns the size in bytes of the block of memory pointed by @a data
		u32 get_size(const void* data)
		{
			ScopedMutex sm(_mutex);
			Header* h = header(data);
			return h->size;
		}
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
	struct ScratchAllocator : public Allocator
	{
		Mutex _mutex;
		Allocator &_backing;

		// Start and end of the ring buffer.
		char*_begin, *_end;

		// Pointers to where to allocate memory and where to free memory.
		char*_allocate, *_free;

		/// Creates a ScratchAllocator. The allocator will use the backing
		/// allocator to create the ring buffer and to service any requests
		/// that don't fit in the ring buffer.
		///
		/// size specifies the size of the ring buffer.
		ScratchAllocator(Allocator &backing, u32 size) : _backing(backing)
		{
			_begin = (char*)_backing.allocate(size);
			_end = _begin + size;
			_allocate = _begin;
			_free = _begin;
		}

		~ScratchAllocator()
		{
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

		void *allocate(u32 size, u32 align)
		{
			ScopedMutex sm(_mutex);

			CE_ASSERT(align % 4 == 0, "Must be 4-byte aligned");
			size = ((size + 3)/4)*4;

			char* p = _allocate;
			Header* h = (Header*)p;
			char* data = (char*)data_pointer(h, align);
			p = data + size;

			// Reached the end of the buffer, wrap around to the beginning.
			if (p > _end) {
				h->size = u32(_end - (char*)h) | 0x80000000u;

				p = _begin;
				h = (Header*)p;
				data = (char*)data_pointer(h, align);
				p = data + size;
			}

			// If the buffer is exhausted use the backing allocator instead.
			if (in_use(p))
				return _backing.allocate(size, align);

			fill(h, data, u32(p - (char*)h));
			_allocate = p;
			return data;
		}

		void deallocate(void *p)
		{
			ScopedMutex sm(_mutex);

			if (!p)
				return;

			if (p < _begin || p >= _end) {
				_backing.deallocate(p);
				return;
			}

			// Mark this slot as free
			Header*h = header(p);
			CE_ASSERT((h->size & 0x80000000u) == 0, "Not free");
			h->size = h->size | 0x80000000u;

			// Advance the free pointer past all free slots.
			while (_free != _allocate) {
				Header*h = (Header*)_free;
				if ((h->size & 0x80000000u) == 0)
					break;

				_free += h->size & 0x7fffffffu;
				if (_free == _end)
					_free = _begin;
			}
		}

		u32 allocated_size(const void *p)
		{
			ScopedMutex sm(_mutex);
			Header* h = header(p);
			return h->size - u32((char*)p - (char*)h);
		}

		u32 total_allocated()
		{
			ScopedMutex sm(_mutex);
			return u32(_end - _begin);
		}
	};

} // namespace memory

namespace memory_globals
{
	using namespace memory;

	static char _buffer[sizeof(HeapAllocator) + sizeof(ScratchAllocator)];
	static HeapAllocator* _default_allocator;
	static ScratchAllocator* _default_scratch_allocator;

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
