#pragma once

#include "Compressor.h"
#include "List.h"

namespace crown
{

class ZipCompressor : Compressor
{
public:
				enum Mode
				{
					DEFAULT,
					LOW,
					HIGH
				};
	
public:

				ZipCompressor(Allocator& allocator);
				~ZipCompressor();
	
	uint8_t* 	compress(const void* data, size_t in_size, size_t& out_size);
	uint8_t* 	uncompress(const void* data, size_t in_size, size_t& out_size);

private:
	
	List<uint8_t> m_data_list;
};

} // namespace crown
