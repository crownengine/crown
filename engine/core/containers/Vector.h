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
#include "Array.h"

namespace crown
{

/// Functions to manipulate Vector.
///
/// @ingroup Containers
namespace vector
{
	/// Returns whether the vector @a v is empty.
	template <typename T> bool empty(const Vector<T>& v);

	/// Returns the number of items in the vector @a v.
	template <typename T> uint32_t size(const Vector<T>& v);

	/// Returns the maximum number of items the vector @a v can hold.
	template <typename T> uint32_t capacity(const Vector<T>& v);

	/// Resizes the vector @a v to the given @a size.
	/// @note
	/// Old items will be copied to the newly created vector.
	/// If the new capacity is smaller than the previous one, the
	/// vector will be truncated.
	template <typename T> void resize(Vector<T>& v, uint32_t size);

	/// Reserves space in the vector @a v for at least @a capacity items.
	template <typename T> void reserve(Vector<T>& v, uint32_t capacity);

	/// Sets the capacity of vector @a v.
	template <typename T> void set_capacity(Vector<T>& v, uint32_t capacity);

	/// Grows the vector @a v to contain at least @a min_capacity items.
	template <typename T> void grow(Vector<T>& v, uint32_t min_capacity);

	/// Condenses the vector @a v so that its capacity matches the actual number
	/// of items in the vector.
	template <typename T> void condense(Vector<T>& v);

	/// Appends an item to the vector @a v and returns its index.
	template <typename T> uint32_t push_back(Vector<T>& v, const T& item);

	/// Removes the last item from the vector @a v.
	template <typename T> void pop_back(Vector<T>& v);

	/// Appends @a count @a items to the vector @a v and returns the number
	/// of items in the vector after the append operation.
	template <typename T> uint32_t push(Vector<T>& v, const T* items, uint32_t count);

	/// Clears the content of the vector @a v.
	/// @note
	/// Calls destructor on the items.
	template <typename T> void clear(Vector<T>& v);

	template <typename T> T* begin(Vector<T>& v);
	template <typename T> const T* begin(const Vector<T>& v);
	template <typename T> T* end(Vector<T>& v);
	template <typename T> const T* end(const Vector<T>& v);

	template <typename T> T& front(Vector<T>& v);
	template <typename T> const T& front(const Vector<T>& v);
	template <typename T> T& back(Vector<T>& v);
	template <typename T> const T& back(const Vector<T>& v);
} // namespace vector

namespace vector
{
	template <typename T>
	bool empty(const Vector<T>& v)
	{
		return array::empty(v.m_array);
	}

	template <typename T>
	uint32_t size(const Vector<T>& v)
	{
		return array::size(v.m_array);
	}

	template <typename T>
	uint32_t capacity(const Vector<T>& v)
	{
		return array::capacity(v.m_array);
	}

	template <typename T>
	void resize(Vector<T>& v, uint32_t size)
	{
		array::resize(v.m_array, size);
	}

	template <typename T>
	void reserve(Vector<T>& v, uint32_t capacity)
	{
		array::reserve(v.m_array, capacity);
	}

	template <typename T>
	void set_capacity(Vector<T>& v, uint32_t capacity)
	{
		if (capacity == v.m_array.m_capacity)
			return;

		if (capacity < v.m_array.m_size)
			resize(v, capacity);

		if (capacity > 0)
		{
			Array<T> arr = v.m_array;

			T* tmp = arr.m_array;
			arr.m_capacity = capacity;

			arr.m_array = (T*)arr.m_allocator->allocate(capacity * sizeof(T));

			for (uint32_t i = 0; i < arr.m_size; i++)
			{
				new (arr.m_array + i) T(tmp[i]);
			}

			if (tmp)
			{
				for (uint32_t i = 0; i < arr.m_size; i++)
				{
					tmp[i].~T();
				}
				arr.m_allocator->deallocate(tmp);
			}
		}
	}

	template <typename T>
	void grow(Vector<T>& v, uint32_t min_capacity)
	{
		return array::grow(v.m_array, min_capacity);
	}

	template <typename T>
	void condense(Vector<T>& v)
	{
		return array::condense(v.m_array);
	}

	template <typename T>
	uint32_t push_back(Vector<T>& v, const T& item)
	{
		if (v.m_array.m_capacity == v.m_array.m_size)
			grow(v, 0);

		new (v.m_array.m_array + v.m_array.m_size) T(item);

		return v.m_array.m_size++;
	}

	template <typename T>
	void pop_back(Vector<T>& v)
	{
		CE_ASSERT(vector::size(v) > 0, "The vector is empty");

		v.m_array.m_array[v.m_array.m_size - 1].~T();
		v.m_array.m_size--;
	}

	template <typename T>
	uint32_t push(Vector<T>& v, const T* items, uint32_t count)
	{
		if (v.m_array.m_capacity <= v.m_array.m_size + count)
			grow(v, v.m_array.m_size + count);

		T* arr = &v.m_array.m_array[v.m_array.m_size];
		for (uint32_t i = 0; i < count; i++)
		{
			arr[i] = items[i];
		}

		v.m_array.m_size += count;
		return v.m_array.m_size;
	}

	template <typename T>
	void clear(Vector<T>& v)
	{
		for (uint32_t i = 0; i < v.m_array.m_size; i++)
		{
			v.m_array.m_array[i].~T();
		}

		v.m_array.m_size = 0;
	}

	template <typename T>
	T* begin(Vector<T>& v)
	{
		return array::begin(v.m_array);
	}
	template <typename T>
	const T* begin(const Vector<T>& v)
	{
		return array::begin(v.m_array);
	}
	template <typename T>
	T* end(Vector<T>& v)
	{
		return array::end(v.m_array);
	}
	template <typename T>
	const T* end(const Vector<T>& v)
	{
		return array::end(v.m_array);
	}

	template <typename T>
	T& front(Vector<T>& v)
	{
		return array::front(v.m_array);
	}
	template <typename T>
	const T& front(const Vector<T>& v)
	{
		return array::front(v.m_array);
	}
	template <typename T>
	T& back(Vector<T>& v)
	{
		return array::back(v.m_array);
	}
	template <typename T>
	const T& back(const Vector<T>& v)
	{
		return array::back(v.m_array);
	}
} // namespace vector

//-----------------------------------------------------------------------------
template <typename T>
inline Vector<T>::Vector(Allocator& allocator)
	: m_array(allocator)
{
}

//-----------------------------------------------------------------------------
template <typename T>
inline Vector<T>::Vector(Allocator& allocator, uint32_t capacity)
	: m_array(allocator)
{
}

//-----------------------------------------------------------------------------
template <typename T>
inline Vector<T>::Vector(const Vector<T>& other)
	: m_array(other.m_array)
{
	*this = other;
}

//-----------------------------------------------------------------------------
template <typename T>
inline Vector<T>::~Vector()
{
	for (uint32_t i = 0; i < array::size(m_array); i++)
	{
		m_array[i].~T();
	}
}

//-----------------------------------------------------------------------------
template <typename T>
inline T& Vector<T>::operator[](uint32_t index)
{
	return m_array[index];
}

//-----------------------------------------------------------------------------
template <typename T>
inline const T& Vector<T>::operator[](uint32_t index) const
{
	return m_array[index];
}

//-----------------------------------------------------------------------------
template <typename T>
inline const Vector<T>& Vector<T>::operator=(const Vector<T>& other)
{
	const uint32_t size = vector::size(other);
	vector::resize(*this, size);

	for (uint32_t i = 0; i < size; i++)
	{
		m_array[i] = other.m_array[i];
	}

	return *this;
}

} // namespace crown
