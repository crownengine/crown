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

#include "pool_allocator.h"
#include "assert.h"

namespace crown
{

PoolAllocator::PoolAllocator(Allocator& backing, size_t num_blocks, size_t block_size, size_t block_align)
	: _backing(backing)
	, _start(NULL)
	, _freelist(NULL)
	, _block_size(block_size)
	, _block_align(block_align)
	, _num_allocations(0)
	, _allocated_size(0)
{
	CE_ASSERT(num_blocks > 0, "Unsupported number of blocks");
	CE_ASSERT(block_size > 0, "Unsupported block size");
	CE_ASSERT(block_align > 0, "Unsupported block alignment");

	size_t actual_block_size = block_size + block_align;
	size_t pool_size = num_blocks * actual_block_size;

	char* mem = (char*) backing.allocate(pool_size, block_align);

	// Initialize intrusive freelist
	char* cur = mem;
	for (size_t bb = 0; bb < num_blocks - 1; bb++)
	{
		uintptr_t* next = (uintptr_t*) cur;
		*next = (uintptr_t) cur + actual_block_size;
		cur += actual_block_size;
	}

	uintptr_t* end = (uintptr_t*) cur;
	*end = (uintptr_t) NULL;

	_start = mem;
	_freelist = mem;
}

PoolAllocator::~PoolAllocator()
{
	_backing.deallocate(_start);
}

void* PoolAllocator::allocate(size_t size, size_t align)
{
	CE_ASSERT(size == _block_size, "Size must match block size");
	CE_ASSERT(align == _block_align, "Align must match block align");
	CE_ASSERT(_freelist != NULL, "Out of memory");

	uintptr_t next_free = *((uintptr_t*) _freelist);
	void* user_ptr = _freelist;
	_freelist = (void*) next_free;

	_num_allocations++;
	_allocated_size += _block_size;

	return user_ptr;
}

void PoolAllocator::deallocate(void* data)
{
	if (!data)
		return;

	CE_ASSERT(_num_allocations > 0, "Did not allocate");

	uintptr_t* next = (uintptr_t*) data;
	*next = (uintptr_t) _freelist;

	_freelist = data;

	_num_allocations--;
	_allocated_size -= _block_size;
}

size_t PoolAllocator::allocated_size()
{
	return _allocated_size;
}

} // namespace crown
