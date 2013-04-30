/*
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
#include "Memory.h"

namespace crown
{

/// Base class for memory allocators.
class Allocator
{
public:

						Allocator() {}
	virtual				~Allocator() {}

	/// Allocates @size bytes of memory aligned to the specified
	/// @align byte and returns a pointer to the first allocated byte.
	virtual void*		allocate(size_t size, size_t align = memory::DEFAULT_ALIGN) = 0;

	/// Deallocates a previously allocated block of memory pointed by @data.
	virtual void		deallocate(void* data) = 0;

	/// Returns the total number of bytes allocated.
	virtual size_t		allocated_size() = 0;

private:

	// Disable copying
						Allocator(const Allocator&);
	Allocator&			operator=(const Allocator&);
};

Allocator& get_default_allocator();

} // namespace crown

