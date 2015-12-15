/*
 * Copyright (c) 2012-2015 Daniele Bartolini and individual contributors.
 * License: https://github.com/taylor001/crown/blob/master/LICENSE
 */

/*
 * Copyright (C) 2012 Bitsquid AB
 * License: https://bitbucket.org/bitsquid/foundation/src/default/LICENCSE
 */

#pragma once

#include "allocator.h"
#include "memory.h"

namespace crown
{

/// Allocates from a fixed-size buffer.
/// When the internal memory is exhausted or the allocation size exceeds
/// the remainig internal storage, the backing allocator is used instead.
/// The memory is automatically freed when the allocator is destroyed.
///
/// @ingroup Memory
template <uint32_t SIZE>
class TempAllocator : public Allocator
{
public:

	/// Uses the @a backing allocator when internal memory is exahusted
	/// or the allocation size exceeds the remaining storage.
	TempAllocator(Allocator& backing = default_scratch_allocator());
	~TempAllocator();

	/// @copydoc Allocator::allocate()
	void* allocate(uint32_t size, uint32_t align = Allocator::DEFAULT_ALIGN);

	/// Does nothing, the memory is automatically freed when the
	/// allocator is destroyed.
	void deallocate(void* /*data*/) {}

	/// @copydoc Allocator::allocated_size()
	uint32_t allocated_size(const void* /*ptr*/) { return SIZE_NOT_TRACKED; }

	/// @copydoc Allocator::total_allocated()
	uint32_t total_allocated() { return SIZE_NOT_TRACKED; }

private:

	Allocator&	_backing;

	char* _begin;
	char* _end;
	char* _cur;
	uint32_t _chunk_size;
	char _buffer[SIZE];
};

typedef TempAllocator<64> TempAllocator64;
typedef TempAllocator<128> TempAllocator128;
typedef TempAllocator<256> TempAllocator256;
typedef TempAllocator<512> TempAllocator512;
typedef TempAllocator<1024> TempAllocator1024;
typedef TempAllocator<2048> TempAllocator2048;
typedef TempAllocator<4096> TempAllocator4096;

template <uint32_t SIZE>
inline TempAllocator<SIZE>::TempAllocator(Allocator& backing)
	: _backing(backing)
	, _begin(&_buffer[0])
	, _end(&_buffer[SIZE - 1])
	, _cur(&_buffer[0])
	, _chunk_size(4 * 1024)
{
	*(void**) _begin = 0;
	_cur += sizeof(void*);
}

template <uint32_t SIZE>
inline TempAllocator<SIZE>::~TempAllocator()
{
	union { char* as_char; void** as_dvoid; };
	as_char = _buffer;

	void *p = *(void **)as_dvoid;
	while (p)
	{
		void *next = *(void **)p;
		_backing.deallocate(p);
		p = next;
	}
}

template <uint32_t SIZE>
inline void* TempAllocator<SIZE>::allocate(uint32_t size, uint32_t align)
{
	_cur = (char*) memory::align_top(_cur, align);

	if (size > uint32_t(_end - _cur))
	{
		uint32_t to_allocate = sizeof(void*) + size + align;

		if (to_allocate < _chunk_size)
		{
			to_allocate = _chunk_size;
		}

		_chunk_size *= 2;

		void *p = _backing.allocate(to_allocate);
		*(void **)_begin = p;
		_cur = _begin = (char*) p;
		_end = _begin + to_allocate;
		*(void**) _begin = 0;
		_cur += sizeof(void*);
		memory::align_top(p, align);
	}

	void *result = _cur;
	_cur += size;
	return result;
}

} // namespace crown
