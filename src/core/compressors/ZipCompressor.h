#pragma once

#include "Compressor.h"
#include "List.h"

namespace crown
{

class ZipCompressor : Compressor
{
public:

				ZipCompressor(Allocator& allocator);
				~ZipCompressor();

	uint8_t*	compress(void* data, size_t in_size, size_t& out_size);
	uint8_t* 	decompress(const void* data, size_t in_size, size_t& out_size);

private:
	
	List<uint8_t> m_data_list;
};

} // namespace crown
