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

#include "types.h"
#include "assert.h"
#include "array.h"

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
		return array::empty(v._array);
	}

	template <typename T>
	uint32_t size(const Vector<T>& v)
	{
		return array::size(v._array);
	}

	template <typename T>
	uint32_t capacity(const Vector<T>& v)
	{
		return array::capacity(v._array);
	}

	template <typename T>
	void resize(Vector<T>& v, uint32_t size)
	{
		array::resize(v._array, size);
	}

	template <typename T>
	void reserve(Vector<T>& v, uint32_t capacity)
	{
		array::reserve(v._array, capacity);
	}

	template <typename T>
	void set_capacity(Vector<T>& v, uint32_t capacity)
	{
		if (capacity == v._array._capacity)
			return;

		if (capacity < v._array._size)
			resize(v, capacity);

		if (capacity > 0)
		{
			Array<T> arr = v._array;

			T* tmp = arr._array;
			arr._capacity = capacity;

			arr._array = (T*)arr._allocator->allocate(capacity * sizeof(T));

			for (uint32_t i = 0; i < arr._size; i++)
			{
				new (arr._array + i) T(tmp[i]);
			}

			if (tmp)
			{
				for (uint32_t i = 0; i < arr._size; i++)
				{
					tmp[i].~T();
				}
				arr._allocator->deallocate(tmp);
			}
		}
	}

	template <typename T>
	void grow(Vector<T>& v, uint32_t min_capacity)
	{
		return array::grow(v._array, min_capacity);
	}

	template <typename T>
	void condense(Vector<T>& v)
	{
		return array::condense(v._array);
	}

	template <typename T>
	uint32_t push_back(Vector<T>& v, const T& item)
	{
		if (v._array._capacity == v._array._size)
			grow(v, 0);

		new (v._array._array + v._array._size) T(item);

		return v._array._size++;
	}

	template <typename T>
	void pop_back(Vector<T>& v)
	{
		CE_ASSERT(vector::size(v) > 0, "The vector is empty");

		v._array._array[v._array._size - 1].~T();
		v._array._size--;
	}

	template <typename T>
	uint32_t push(Vector<T>& v, const T* items, uint32_t count)
	{
		if (v._array._capacity <= v._array._size + count)
			grow(v, v._array._size + count);

		T* arr = &v._array._array[v._array._size];
		for (uint32_t i = 0; i < count; i++)
		{
			arr[i] = items[i];
		}

		v._array._size += count;
		return v._array._size;
	}

	template <typename T>
	void clear(Vector<T>& v)
	{
		for (uint32_t i = 0; i < v._array._size; i++)
		{
			v._array._array[i].~T();
		}

		v._array._size = 0;
	}

	template <typename T>
	T* begin(Vector<T>& v)
	{
		return array::begin(v._array);
	}
	template <typename T>
	const T* begin(const Vector<T>& v)
	{
		return array::begin(v._array);
	}
	template <typename T>
	T* end(Vector<T>& v)
	{
		return array::end(v._array);
	}
	template <typename T>
	const T* end(const Vector<T>& v)
	{
		return array::end(v._array);
	}

	template <typename T>
	T& front(Vector<T>& v)
	{
		return array::front(v._array);
	}
	template <typename T>
	const T& front(const Vector<T>& v)
	{
		return array::front(v._array);
	}
	template <typename T>
	T& back(Vector<T>& v)
	{
		return array::back(v._array);
	}
	template <typename T>
	const T& back(const Vector<T>& v)
	{
		return array::back(v._array);
	}
} // namespace vector

template <typename T>
inline Vector<T>::Vector(Allocator& allocator)
	: _array(allocator)
{
}

template <typename T>
inline Vector<T>::Vector(Allocator& allocator, uint32_t capacity)
	: _array(allocator)
{
}

template <typename T>
inline Vector<T>::Vector(const Vector<T>& other)
	: _array(other._array)
{
	*this = other;
}

template <typename T>
inline Vector<T>::~Vector()
{
	for (uint32_t i = 0; i < array::size(_array); i++)
	{
		_array[i].~T();
	}
}

template <typename T>
inline T& Vector<T>::operator[](uint32_t index)
{
	return _array[index];
}

template <typename T>
inline const T& Vector<T>::operator[](uint32_t index) const
{
	return _array[index];
}

template <typename T>
inline const Vector<T>& Vector<T>::operator=(const Vector<T>& other)
{
	const uint32_t size = vector::size(other);
	vector::resize(*this, size);

	for (uint32_t i = 0; i < size; i++)
	{
		_array[i] = other._array[i];
	}

	return *this;
}

} // namespace crown
