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
#include "macros.h"

namespace crown
{

/// Offers the facility to tag allocators by a string identifier.
/// Proxy allocator is appended to a global linked list when instantiated
/// so that it is possible to later visit that list for debugging purposes.
///
/// @ingroup Memory
class ProxyAllocator : public Allocator
{
public:

	/// Tag all allocations made with @a allocator by the given @a name
	ProxyAllocator(const char* name, Allocator& allocator);

	/// @copydoc Allocator::allocate()
	void* allocate(size_t size, size_t align = Allocator::DEFAULT_ALIGN);

	/// @copydoc Allocator::deallocate()
	void deallocate(void* data);

	/// @copydoc Allocator::allocated_size()
	size_t allocated_size();

	/// Returns the name of the proxy allocator
	const char* name() const;

public:

	/// Returns the total number of proxy allocators.
	/// in the global list.
	static uint32_t count();

	/// Returns the proxy allocator @name or NULL if not found.
	static ProxyAllocator* find(const char* name);

	/// Returns the first proxy allocator in the global list or
	/// NULL if the list is empty.
	static ProxyAllocator* begin();

	/// Returns the next proxy allocator to @a a in the global list
	/// or NULL if end-of-list is reached.
	static ProxyAllocator* next(ProxyAllocator* a);

private:

	Allocator& m_allocator;
	const char* m_name;
	size_t m_total_allocated;

	ProxyAllocator* m_next;
};

} // namespace crown
