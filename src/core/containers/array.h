/*
 * Copyright (c) 2012-2015 Daniele Bartolini and individual contributors.
 * License: https://github.com/taylor001/crown/blob/master/LICENSE
 */

#pragma once

#include "error.h"
#include "macros.h"
#include "container_types.h"
#include "allocator.h"
#include <cstring> // memcpy

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
		return a._size == 0;
	}

	template <typename T>
	inline uint32_t size(const Array<T>& a)
	{
		return a._size;
	}

	template <typename T>
	inline uint32_t capacity(const Array<T>& a)
	{
		return a._capacity;
	}

	template <typename T>
	inline void resize(Array<T>& a, uint32_t size)
	{
		if (size > a._capacity)
			set_capacity(a, size);

		a._size = size;
	}

	template <typename T>
	inline void reserve(Array<T>& a, uint32_t capacity)
	{
		if (capacity > a._capacity)
			grow(a, capacity);
	}

	template <typename T>
	inline void set_capacity(Array<T>& a, uint32_t capacity)
	{
		if (capacity == a._capacity)
			return;

		if (capacity < a._size)
			resize(a, capacity);

		if (capacity > 0)
		{
			T* tmp = a._array;
			a._capacity = capacity;
			a._array = (T*)a._allocator->allocate(capacity * sizeof(T), CE_ALIGNOF(T));

			memcpy(a._array, tmp, a._size * sizeof(T));

			if (tmp)
				a._allocator->deallocate(tmp);
		}
	}

	template <typename T>
	inline void grow(Array<T>& a, uint32_t min_capacity)
	{
		uint32_t new_capacity = a._capacity * 2 + 1;

		if (new_capacity < min_capacity)
			new_capacity = min_capacity;

		set_capacity(a, new_capacity);
	}

	template <typename T>
	inline void condense(Array<T>& a)
	{
		resize(a, a._size);
	}

	template <typename T>
	inline uint32_t push_back(Array<T>& a, const T& item)
	{
		if (a._capacity == a._size)
			grow(a, 0);

		a._array[a._size] = item;

		return a._size++;
	}

	template <typename T>
	inline void pop_back(Array<T>& a)
	{
		CE_ASSERT(a._size > 0, "The array is empty");

		--a._size;
	}

	template <typename T>
	inline uint32_t push(Array<T>& a, const T* items, uint32_t count)
	{
		if (a._capacity <= a._size + count)
			grow(a, a._size + count);

		memcpy(&a._array[a._size], items, sizeof(T) * count);
		a._size += count;

		return a._size;
	}

	template <typename T>
	inline void clear(Array<T>& a)
	{
		a._size = 0;
	}

	template <typename T>
	inline const T* begin(const Array<T>& a)
	{
		return a._array;
	}

	template <typename T>
	inline T* begin(Array<T>& a)
	{
		return a._array;
	}

	template <typename T>
	inline const T* end(const Array<T>& a)
	{
		return a._array + a._size;
	}

	template <typename T>
	inline T* end(Array<T>& a)
	{
		return a._array + a._size;
	}

	template <typename T>
	inline T& front(Array<T>& a)
	{
		CE_ASSERT(a._size > 0, "The array is empty");

		return a._array[0];
	}

	template <typename T>
	inline const T& front(const Array<T>& a)
	{
		CE_ASSERT(a._size > 0, "The array is empty");

		return a._array[0];
	}

	template <typename T>
	inline T& back(Array<T>& a)
	{
		CE_ASSERT(a._size > 0, "The array is empty");

		return a._array[a._size - 1];
	}

	template <typename T>
	inline const T& back(const Array<T>& a)
	{
		CE_ASSERT(a._size > 0, "The array is empty");

		return a._array[a._size - 1];
	}
} // namespace array

template <typename T>
inline Array<T>::Array(Allocator& a)
	: _allocator(&a)
	, _capacity(0)
	, _size(0)
	, _array(NULL)
{
}

template <typename T>
inline Array<T>::Array(Allocator& a, uint32_t capacity)
	: _allocator(&a)
	, _capacity(0)
	, _size(0)
	, _array(NULL)
{
	array::resize(*this, capacity);
}

template <typename T>
inline Array<T>::Array(const Array<T>& other)
	: _allocator(other._allocator)
	, _capacity(0)
	, _size(0)
	, _array(NULL)
{
	*this = other;
}

template <typename T>
inline Array<T>::~Array()
{
	if (_array)
		_allocator->deallocate(_array);
}

template <typename T>
inline T& Array<T>::operator[](uint32_t index)
{
	CE_ASSERT(index < _size, "Index out of bounds");

	return _array[index];
}

template <typename T>
inline const T& Array<T>::operator[](uint32_t index) const
{
	CE_ASSERT(index < _size, "Index out of bounds");

	return _array[index];
}

template <typename T>
inline Array<T>& Array<T>::operator=(const Array<T>& other)
{
	const uint32_t size = other._size;
	array::resize(*this, size);
	memcpy(_array, other._array, sizeof(T) * size);
	return *this;
}

} // namespace crown
