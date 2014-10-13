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

#include "linear_allocator.h"
#include "memory.h"

namespace crown
{

//-----------------------------------------------------------------------------
LinearAllocator::LinearAllocator(Allocator& backing, size_t size)
	: _backing(&backing)
	, _physical_start(NULL)
	, _total_size(size)
	, _offset(0)
{
	_physical_start = backing.allocate(size);
}

//-----------------------------------------------------------------------------
LinearAllocator::LinearAllocator(void* start, size_t size)
	: _backing(NULL)
	, _physical_start(start)
	, _total_size(size)
	, _offset(0)
{
}

//-----------------------------------------------------------------------------
LinearAllocator::~LinearAllocator()
{
	if (_backing)
	{
		_backing->deallocate(_physical_start);
	}

	CE_ASSERT(_offset == 0, "Memory leak of %ld bytes, maybe you forgot to call clear()?", _offset);
}

//-----------------------------------------------------------------------------
void* LinearAllocator::allocate(size_t size, size_t align)
{
	const size_t actual_size = size + align;

	// Memory exhausted
	if (_offset + actual_size > _total_size)
	{
		return NULL;
	}

	void* user_ptr = memory::align_top((char*) _physical_start + _offset, align);

	_offset += actual_size;

	return user_ptr;
}

//-----------------------------------------------------------------------------
void LinearAllocator::deallocate(void* /*data*/)
{
	// Single deallocations not supported. Use clear().
}

//-----------------------------------------------------------------------------
void LinearAllocator::clear()
{
	_offset = 0;
}

//-----------------------------------------------------------------------------
size_t LinearAllocator::allocated_size()
{
	return _offset;
}

} // namespace crown
