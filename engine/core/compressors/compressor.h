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

#pragma once

#include "types.h"
#include "allocator.h"

namespace crown
{

class Compressor
{
public:
						Compressor(Allocator& allocator);
	virtual 			~Compressor() {}

	/// Compresses the data pointed by @a data of size @a in_size (in bytes) and returns it.
	/// The function also returns the size in bytes of the compressed data in @a out_size.
	/// @note
	/// The returned data is automatically allocated using the passed allocator.
	virtual uint8_t* 	compress(const void* data, size_t in_size, size_t& out_size) = 0;

	/// Uncompresses the data pointed by @a data of size @a in_size (in bytes) and returns it.
	/// The function also returns the size in bytes of the uncompressed data in @a out_size.
	/// @note
	/// The returned data is automatically allocated using the passed allocator.
	virtual uint8_t* 	uncompress(const void* data, size_t in_size, size_t& out_size) = 0;

protected:

	Allocator& 			m_allocator;
};

//-----------------------------------------------------------------------------
inline Compressor::Compressor(Allocator& allocator) :
	m_allocator(allocator)
{
}

} // namespace crown
