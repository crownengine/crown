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

#include "allocator.h"

namespace crown
{

/// Allocates memory linearly from a predefined chunk
/// and frees all the allocations with a single call to clear()
///
/// @ingroup Memory
class LinearAllocator : public Allocator
{
public:

	LinearAllocator(Allocator& backing, size_t size);
	LinearAllocator(void* start, size_t size);
	~LinearAllocator();

	/// @copydoc Allocator::allocate()
	void* allocate(size_t size, size_t align = Allocator::DEFAULT_ALIGN);

	/// @copydoc Allocator::deallocate()
	/// @note
	/// The linear allocator does not support deallocating
	/// individual allocations, rather you have to call
	/// clear() to free all allocated memory at once.
	void deallocate(void* data);

	/// Frees all the allocations made by allocate()
	void clear();

	/// @copydoc Allocator::allocated_size()
	size_t allocated_size();

private:

	Allocator* _backing;
	
	void* _physical_start;
	size_t _total_size;
	size_t _offset;
};

} // namespace crown
