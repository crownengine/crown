#include <cstdio>
#include <cstring>
#include "MallocAllocator.h"
#include "ZipCompressor.h"

using namespace crown;

int main(int argc, char** argv)
{
	if (argc != 2)
	{
		printf("Arguments must be 2");
		return -1;
	}
	
	MallocAllocator allocator;
	ZipCompressor compressor(allocator);

	char* file_name = argv[1];

	const char* uncompressed_string = "foobar";
	uint8_t* result;
	size_t result_size = 0;

	result = compressor.compress((void*)uncompressed_string, strlen(uncompressed_string), result_size);

	printf("Before: ");
	printf("Size: %d", strlen(uncompressed_string));
	printf(uncompressed_string);
	printf("\n");

	printf("After: ");
	printf("Size: %d", result_size);
	for (size_t i = 0; i < result_size; i++)
	{
		printf("%c", result[i]);
	}
	printf("\n");

//	allocator->deallocate(result);
	
	return 0;
}
