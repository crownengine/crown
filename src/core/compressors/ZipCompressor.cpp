/*
Copyright (c) 2013 Daniele Bartolini, Michele Rossi
Copyright (c) 2012 Daniele Bartolini, Simone Boscaratto

Permission is hereby granted, free of charge, to any person
obtaining a copy of this software and associated documentation
files (the "Software"), to deal in the Software without
restriction, including without limitation the rights to use,
copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the
Software is furnished to do so, subject to the following
conditions:

The above copyright notice and this permission notice shall be
included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
OTHER DEALINGS IN THE SOFTWARE.
*/

#include <cstring>

#include "zlib.h"
#include "Assert.h"
#include "ZipCompressor.h"

namespace crown
{

//-----------------------------------------------------------------------------
ZipCompressor::ZipCompressor(Allocator& allocator) :
	Compressor(allocator)
{
}

//-----------------------------------------------------------------------------
ZipCompressor::~ZipCompressor()
{
}

//-----------------------------------------------------------------------------
uint8_t* ZipCompressor::compress(const void* data, size_t in_size, size_t& out_size)
{
	out_size = in_size + in_size * 0.1f + 12;
	
 	uint8_t* dest = (uint8_t*)m_allocator.allocate(out_size);
	
	int32_t ret = ::compress((Bytef*)dest, (uLongf*)&out_size, (const Bytef*)data, (uLongf)in_size);
	
	CE_ASSERT(ret == Z_OK, "Failed to compress the data");
	
	return dest;
}

//-----------------------------------------------------------------------------
uint8_t* ZipCompressor::uncompress(const void* data, size_t in_size, size_t& out_size)
{
	out_size = in_size + in_size * 0.1f + 12;
	
 	uint8_t* dest = (uint8_t*)m_allocator.allocate(out_size);
	
	int32_t ret = ::uncompress((Bytef*)dest, (uLongf*)&out_size, (const Bytef*)data, (uLongf)in_size);
	
	CE_ASSERT(ret == Z_OK, "Failed to uncompress the data");
	
	return dest;
}

} // namespace crown

