#include <cstdio>
#include <cstring>
#include "MallocAllocator.h"
#include "ZipCompressor.h"

using namespace crown;

int main(int argc, char** argv)
{
	
	MallocAllocator allocator;
	ZipCompressor compressor(allocator);

	const char* uncompressed_string = "letstry";
	uint8_t* compressed_string;
	uint8_t* result;
	size_t compr_size = 0;
	size_t result_size = 0;
	
 	compressed_string = compressor.compress((void*)uncompressed_string, strlen(uncompressed_string), compr_size);
	
	printf("Uncompressed: ");
	printf("Size: %d - ", strlen(uncompressed_string));
	printf(uncompressed_string);
	printf("\n\n");

	printf("Compressed: ");
	printf("Size: %d - ", compr_size);
	for (size_t i = 0; i < compr_size; i++)
	{
		printf("%c", compressed_string[i]);
	}
	printf("\n\n");

	result = compressor.uncompress((void*)compressed_string, compr_size, result_size);
	
	printf("Uncompressed again: ");
	printf("Size: %d - ", result_size);
	for (size_t i = 0; i < result_size; i++)
	{
		printf("%c", result[i]);
	}
	printf("\n\n");
	
	allocator.deallocate(compressed_string);
  	allocator.deallocate(result); 
	
	return 0;
}
