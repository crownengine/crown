#include <cstring>

#include "zlib.h"
#include "ZipCompressor.h"

namespace crown
{

ZipCompressor::ZipCompressor(Allocator& allocator) :
	Compressor(allocator)
{
	
}

ZipCompressor::~ZipCompressor()
{

}

uint8_t* ZipCompressor::compress(const void* data, size_t in_size, size_t& out_size)
{
	out_size = in_size + in_size * 0.1f + 12;
	
 	uint8_t* dest = (uint8_t*)m_allocator.allocate(out_size);
	
	int32_t ret = ::compress((Bytef*)dest, (uLongf*)&out_size, (const Bytef*)data, (uLongf)in_size);
	
	assert(ret == Z_OK);
	
	return dest;
}

uint8_t* ZipCompressor::uncompress(const void* data, size_t in_size, size_t& out_size)
{
	out_size = in_size + in_size * 0.1f + 12;
	
 	uint8_t* dest = (uint8_t*)m_allocator.allocate(out_size);
	
	int32_t ret = ::uncompress((Bytef*)dest, (uLongf*)&out_size, (const Bytef*)data, (uLongf)in_size);
	
	assert(ret == Z_OK);
	
	return dest;
}

} // namespace crown

