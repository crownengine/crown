#pragma once

#include "Types.h"
#include "Allocator.h"

namespace crown
{

class Compressor
{
public:
						Compressor(Allocator& allocator);
	virtual 			~Compressor();	

	virtual uint8_t* 	compress(const void* data, size_t in_size, size_t& out_size) = 0;
	virtual uint8_t* 	uncompress(const void* data, size_t in_size, size_t& out_size) = 0;

protected:
	
	Allocator& 			m_allocator;
};

inline Compressor::Compressor(Allocator& allocator) :
	m_allocator(allocator)
{

}

inline Compressor::~Compressor()
{
	
}

}
