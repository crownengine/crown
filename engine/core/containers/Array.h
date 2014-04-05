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

#include <cstring>
#include "Assert.h"
#include "ContainerTypes.h"

namespace crown
{

/// Functions to manipulate Array.
///
/// @ingroup Containers
namespace array
{
	/// Returns whether the array @a a is empty.
	template <typename T> bool empty(const Array<T>& a);

	/// Returns the number of items in the array @a a.
	template <typename T> uint32_t size(const Array<T>& a);

	/// Returns the maximum number of items the array @a a can hold.
	template <typename T> uint32_t capacity(const Array<T>& a);

	/// Resizes the array @a a to the given @a size.
	/// @note
	/// Old items will be copied to the newly created array.
	/// If the new capacity is smaller than the previous one, the
	/// array will be truncated.
	template <typename T> void resize(Array<T>& a, uint32_t size);

	/// Reserves space in the array @a a for at least @a capacity items.
	template <typename T> void reserve(uint32_t capacity);

	/// Sets the capacity of array @a a.
	template <typename T> void set_capacity(Array<T>& a, uint32_t capacity);

	/// Grows the array @a a to contain at least @a min_capacity items.
	template <typename T> void grow(Array<T>& a, uint32_t min_capacity);

	/// Condenses the array @a a so that its capacity matches the actual number
	/// of items in the array.
	template <typename T> void condense(Array<T>& a);

	/// Appends an item to the array @a a and returns its index.
	template <typename T> uint32_t push_back(Array<T>& a, const T& item);

	/// Removes the last item from the array @a a.
	template <typename T> void pop_back(Array<T>& a);

	/// Appends @a count @a items to the array @a a and returns the number
	/// of items in the array after the append operation.
	template <typename T> uint32_t push(Array<T>& a, const T* items, uint32_t count);

	/// Clears the content of the array @a a.
	/// @note
	/// Does not free memory nor call destructors, it only zeroes
	/// the number of items in the array.
	template <typename T> void clear(Array<T>& a);

	template <typename T> T* begin(Array<T>& a);
	template <typename T> const T* begin(const Array<T>& a);
	template <typename T> T* end(Array<T>& a);
	template <typename T> const T* end(const Array<T>& a);

	template <typename T> T& front(Array<T>& a);
	template <typename T> const T& front(const Array<T>& a);
	template <typename T> T& back(Array<T>& a);
	template <typename T> const T& back(const Array<T>& a);
} // namespace array

namespace array
{
	template <typename T>
	inline bool empty(const Array<T>& a)
	{
		return a.m_size == 0;
	}

	template <typename T>
	inline uint32_t size(const Array<T>& a)
	{
		return a.m_size;
	}

	template <typename T>
	inline uint32_t capacity(const Array<T>& a)
	{
		return a.m_capacity;
	}

	template <typename T>
	inline void resize(Array<T>& a, uint32_t size)
	{
		if (size > a.m_capacity)
		{
			set_capacity(a, size);
		}

		a.m_size = size;
	}

	template <typename T>
	inline void reserve(Array<T>& a, uint32_t capacity)
	{
		if (capacity > a.m_capacity)
		{
			grow(a, capacity);
		}
	}

	template <typename T>
	inline void set_capacity(Array<T>& a, uint32_t capacity)
	{
		if (capacity == a.m_capacity)
		{
			return;
		}

		if (capacity < a.m_size)
		{
			resize(a, capacity);
		}

		if (capacity > 0)
		{
			T* tmp = a.m_array;
			a.m_capacity = capacity;

			a.m_array = (T*)a.m_allocator->allocate(capacity * sizeof(T), CE_ALIGNOF(T));

			memcpy(a.m_array, tmp, a.m_size * sizeof(T));

			if (tmp)
			{
				a.m_allocator->deallocate(tmp);
			}
		}
	}

	template <typename T>
	inline void grow(Array<T>& a, uint32_t min_capacity)
	{
		uint32_t new_capacity = a.m_capacity * 2 + 1;

		if (new_capacity < min_capacity)
		{
			new_capacity = min_capacity;
		}

		set_capacity(a, new_capacity);
	}

	template <typename T>
	inline void condense(Array<T>& a)
	{
		resize(a, a.m_size);
	}

	template <typename T>
	inline uint32_t push_back(Array<T>& a, const T& item)
	{
		if (a.m_capacity == a.m_size)
		{
			grow(a, 0);
		}

		a.m_array[a.m_size] = item;

		return 	a.m_size++;
	}

	template <typename T>
	inline void pop_back(Array<T>& a)
	{
		CE_ASSERT(a.m_size > 0, "The array is empty");

		a.m_size--;
	}

	template <typename T>
	inline uint32_t push(Array<T>& a, const T* items, uint32_t count)
	{
		if (a.m_capacity <= a.m_size + count)
		{
			grow(a, a.m_size + count);
		}

		memcpy(&a.m_array[a.m_size], items, sizeof(T) * count);
		a.m_size += count;

		return a.m_size;
	}

	template <typename T>
	inline void clear(Array<T>& a)
	{
		a.m_size = 0;
	}

	template <typename T>
	inline const T* begin(const Array<T>& a)
	{
		return a.m_array;
	}

	template <typename T>
	inline T* begin(Array<T>& a)
	{
		return a.m_array;
	}

	template <typename T>
	inline const T* end(const Array<T>& a)
	{
		return a.m_array + a.m_size;
	}

	template <typename T>
	inline T* end(Array<T>& a)
	{
		return a.m_array + a.m_size;
	}

	template <typename T>
	inline T& front(Array<T>& a)
	{
		CE_ASSERT(a.m_size > 0, "The array is empty");

		return a.m_array[0];
	}

	template <typename T>
	inline const T& front(const Array<T>& a)
	{
		CE_ASSERT(a.m_size > 0, "The array is empty");

		return a.m_array[0];
	}

	template <typename T>
	inline T& back(Array<T>& a)
	{
		CE_ASSERT(a.m_size > 0, "The array is empty");

		return a.m_array[a.m_size - 1];
	}

	template <typename T>
	inline const T& back(const Array<T>& a)
	{
		CE_ASSERT(a.m_size > 0, "The array is empty");

		return a.m_array[a.m_size - 1];
	}
} // namespace array

template <typename T>
inline Array<T>::Array(Allocator& allocator)
	: m_allocator(&allocator), m_capacity(0), m_size(0), m_array(NULL)
{
}

template <typename T>
inline Array<T>::Array(Allocator& allocator, uint32_t capacity)
	: m_allocator(&allocator), m_capacity(0), m_size(0), m_array(NULL)
{
	array::resize(*this, capacity);
}

template <typename T>
inline Array<T>::Array(const Array<T>& other)
	: m_allocator(other.m_allocator), m_capacity(0), m_size(0), m_array(NULL)
{
	*this = other;
}

template <typename T>
inline Array<T>::~Array()
{
	if (m_array)
	{
		m_allocator->deallocate(m_array);
	}
}

template <typename T>
inline T& Array<T>::operator[](uint32_t index)
{
	CE_ASSERT(index < m_size, "Index out of bounds");

	return m_array[index];
}

template <typename T>
inline const T& Array<T>::operator[](uint32_t index) const
{
	CE_ASSERT(index < m_size, "Index out of bounds");

	return m_array[index];
}

template <typename T>
inline Array<T>& Array<T>::operator=(const Array<T>& other)
{
	const uint32_t size = other.m_size;
	array::resize(*this, size);
	memcpy(m_array, other.m_array, sizeof(T) * size);
	return *this;
}

} // namespace crown
