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
#include "Allocator.h"

namespace crown
{

/// @defgroup Containers Containers

/// Dynamic array of POD items.
/// @note
/// Does not call constructors/destructors so it is not very suitable for non-POD items.
///
/// @ingroup Containers
template <typename T>
struct Array
{
	Array(Allocator& allocator);

	/// Allocates capacity * sizeof(T) bytes.
	Array(Allocator& allocator, uint32_t capacity);
	Array(const Array<T>& other);
	~Array();

	/// Random access by index
	T& operator[](uint32_t index);
	const T& operator[](uint32_t index) const;

	Array<T>& operator=(const Array<T>& other);

	Allocator* m_allocator;
	uint32_t m_capacity;
	uint32_t m_size;
	T* m_array;
};

/// Circular buffer double-ended queue of POD items.
/// @note
/// Does not call constructors/destructors so it is not very suitable for non-POD items.
///
/// @ingroup Containers
template <typename T>
struct Queue
{
	Queue(Allocator& allocator);

	/// Random access by index
	T& operator[](uint32_t index);

	/// Random access by index
	const T& operator[](uint32_t index) const;

	uint32_t m_read;
	uint32_t m_size;
	Array<T> m_queue;
};

/// Priority queue of POD items.
///
/// @ingroup Containers
template <typename T>
struct PriorityQueue
{
	PriorityQueue(Allocator& a);

	Array<T> m_queue;
};

/// Hash from an uint64_t to POD items. If you want to use a generic key
/// item, use a hash function to map that item to an uint64_t.
///
/// @ingroup Containers
template<typename T>
struct Hash
{
	Hash(Allocator &a);
	
	struct Entry
	{
		uint64_t key;
		uint32_t next;
		T value;
	};

	Array<uint32_t> _hash;
	Array<Entry> _data;
};

} // namespace crown
