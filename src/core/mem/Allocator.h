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

#include <new>

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

	/// Allocates @a size bytes of memory aligned to the specified
	/// @a align byte and returns a pointer to the first allocated byte.
	virtual void*		allocate(size_t size, size_t align = memory::DEFAULT_ALIGN) = 0;

	/// Deallocates a previously allocated block of memory pointed by @a data.
	virtual void		deallocate(void* data) = 0;

	/// Returns the total number of bytes allocated.
	virtual size_t		allocated_size() = 0;

private:

	// Disable copying
						Allocator(const Allocator&);
	Allocator&			operator=(const Allocator&);
};

Allocator& default_allocator();

/// Respects standard behaviour when calling on NULL @a ptr
template <typename T>
void call_destructor_and_deallocate(Allocator& a, T* ptr)
{
	if (ptr != NULL)
	{
		ptr->~T();

		a.deallocate(ptr);
	}
}

/// Allocates memory with @a allocator for the given @a T type
/// and calls constructor on it.
/// @note
/// @a allocator must be a reference to an existing allocator.
#define CE_NEW(allocator, T)\
	new ((allocator).allocate(sizeof(T), CE_ALIGNOF(T))) T

/// Calls destructor on @a ptr and deallocates memory using the
/// given @a allocator.
/// @note
/// @a allocator must be a reference to an existing allocator.
#define CE_DELETE(allocator, ptr)\
	call_destructor_and_deallocate(allocator, ptr)

} // namespace crown
