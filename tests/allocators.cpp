#include "Allocator.h"
#include "MallocAllocator.h"
#include <cstdio>
#include <cassert>

using namespace crown;

int main()
{
	MallocAllocator malloc_allocator;

	char* char_buffer = (char*)malloc_allocator.allocate(128);
	assert(malloc_allocator.allocated_size() >= 128);

	printf("MallocAllocator::get_allocated_size(): %d\n", malloc_allocator.allocated_size());

	malloc_allocator.deallocate(char_buffer);

	printf("MallocAllocator::get_allocated_size(): %d\n", malloc_allocator.allocated_size());
	//assert(malloc_allocator.get_allocated_size() == 0);
}

