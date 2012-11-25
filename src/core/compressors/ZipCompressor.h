#pragma once

#include "Compressor.h"
#include "List.h"

namespace crown
{

class ZipCompressor : public Compressor
{
public:

				ZipCompressor(Allocator& allocator);
				~ZipCompressor();
	
	uint8_t* 	compress(const void* data, size_t in_size, size_t& out_size);
	uint8_t* 	uncompress(const void* data, size_t in_size, size_t& out_size);
};

} // namespace crown
