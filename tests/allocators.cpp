#include "Allocator.h"
#include "MallocAllocator.h"
#include <cstdio>
#include "Assert.h"

using namespace crown;

int main()
{
	MallocAllocator malloc_allocator;

	char* char_buffer = (char*)malloc_allocator.allocate(128);
	CE_ASSERT(malloc_allocator.allocated_size() >= 128, "Allocated size differs from requested size");

	printf("MallocAllocator::get_allocated_size(): %d\n", malloc_allocator.allocated_size());

	malloc_allocator.deallocate(char_buffer);

	printf("MallocAllocator::get_allocated_size(): %d\n", malloc_allocator.allocated_size());
	//CE_ASSERT(malloc_allocator.get_allocated_size() == 0);
}

