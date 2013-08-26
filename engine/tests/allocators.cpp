#include "Allocator.h"
#include "HeapAllocator.h"
#include "StackAllocator.h"
#include <cstdio>
#include "Assert.h"

using namespace crown;

int main()
{
	HeapAllocator malloc_allocator;

	char* char_buffer = (char*)malloc_allocator.allocate(128);
	CE_ASSERT(malloc_allocator.allocated_size() >= 128, "Allocated size differs from requested size");

	printf("HeapAllocator::get_allocated_size(): %d\n", (uint32_t)malloc_allocator.allocated_size());

	malloc_allocator.deallocate(char_buffer);

	printf("HeapAllocator::get_allocated_size(): %d\n", (uint32_t)malloc_allocator.allocated_size());
	//CE_ASSERT(malloc_allocator.get_allocated_size() == 0);

	uint8_t buffer[1024 * 1024];

	StackAllocator stack(buffer, 1024 * 1024);

	stack.allocate(12);
	stack.allocate(5);

	memory::dump(buffer, stack.allocated_size(), 4);
}

