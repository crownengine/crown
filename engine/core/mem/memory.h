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
#include "types.h"
#include "assert.h"
#include "allocator.h"

namespace crown
{

Allocator& default_allocator();

/// @defgroup Memory Memory
namespace memory
{

/// Value used to fill unused memory
const uint32_t PADDING_VALUE = 0xFFFFFFFFu;

/// Constructs the initial default allocators.
/// @note
/// Has to be called before anything else during the engine startup.
void init();

/// Destroys the allocators created with memory::init().
/// @note
/// Should be the last call of the program.
void shutdown();

/// Returns the pointer @a p aligned to the desired @a align byte
inline void* align_top(void* p, size_t align)
{
	CE_ASSERT(align >= 1, "Alignment must be > 1");
	CE_ASSERT(align % 2 == 0 || align == 1, "Alignment must be a power of two");

	uintptr_t ptr = (uintptr_t)p;

	const size_t mod = ptr % align;

	if (mod)
	{
		ptr += align - mod;
	}

	return (void*) ptr;
}

/// Respects standard behaviour when calling on NULL @a ptr
template <typename T>
inline void call_destructor_and_deallocate(Allocator& a, T* ptr)
{
	if (!ptr)
		return;

	ptr->~T();
	a.deallocate(ptr);
}

/// Allocates memory with @a allocator for the given @a T type
/// and calls constructor on it.
/// @note
/// @a allocator must be a reference to an existing allocator.
#define CE_NEW(allocator, T) new ((allocator).allocate(sizeof(T), CE_ALIGNOF(T))) T

/// Calls destructor on @a ptr and deallocates memory using the
/// given @a allocator.
/// @note
/// @a allocator must be a reference to an existing allocator.
#define CE_DELETE(allocator, ptr) crown::memory::call_destructor_and_deallocate(allocator, ptr)
} // namespace memory
} // namespace crown
