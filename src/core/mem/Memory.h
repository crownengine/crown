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

#include "Types.h"
#include "Assert.h"
#include "OS.h"

namespace crown
{
namespace memory
{

const uint32_t	PADDING_VALUE	= 0xFFFFFFFFu;	//!< Value used to fill unused memory
const size_t	DEFAULT_ALIGN	= 4;			//!< Default memory alignment in bytes

/// Returns the pointer @a p aligned to the desired @a align byte
inline void* align_top(void* p, size_t align)
{
	CE_ASSERT(align > 1, "Alignment must be > 1");
	CE_ASSERT(align % 2 == 0, "Alignment must be a power of two");

	uintptr_t ptr = (uintptr_t)p;

	const size_t mod = ptr % align;

	if (mod)
	{
		ptr += align - mod;
	}

	return (void*) ptr;
}

/// Dumps the memory content pointed by @a p
inline void dump(void* p, size_t size, size_t word_size)
{
	uint8_t* mem = (uint8_t*) p;

	for (size_t i = 0; i < size; i++, mem++)
	{
		if (i % word_size == 0)
		{
			os::printf("\n");
			os::printf("[.. %.4X] ", (size_t)mem);
		}

		os::printf("%.2X ", *mem);
	}

	os::printf("\n");
}

} // namespace memory
} // namespace crown
