/*
 * Copyright (c) 2012-2021 Daniele Bartolini et al.
 * License: https://github.com/dbartolini/crown/blob/master/LICENSE
 */

#pragma once

#include "core/containers/types.h"
#include "core/error/error.inl"
#include "core/memory/allocator.h"
#include <string.h> // memcpy

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
	template <typename T> u32 size(const Array<T>& a);

	/// Returns the maximum number of items the array @a a can hold.
	template <typename T> u32 capacity(const Array<T>& a);

	/// Resizes the array @a a to the given @a size.
	/// @note
	/// Old items will be copied to the newly created array.
	/// If the new capacity is smaller than the previous one, the
	/// array will be truncated.
	template <typename T> void resize(Array<T>& a, u32 size);

	/// Reserves space in the array @a a for at least @a capacity items.
	template <typename T> void reserve(Array<T>& a, u32 capacity);

	/// Sets the capacity of array @a a.
	template <typename T> void set_capacity(Array<T>& a, u32 capacity);

	/// Grows the array @a a to contain at least @a min_capacity items.
	template <typename T> void grow(Array<T>& a, u32 min_capacity);

	/// Condenses the array @a a so that its capacity matches the actual number
	/// of items in the array.
	template <typename T> void condense(Array<T>& a);

	/// Appends an item to the array @a a and returns its index.
	template <typename T> u32 push_back(Array<T>& a, const T& item);

	/// Removes the last item from the array @a a.
	template <typename T> void pop_back(Array<T>& a);

	/// Appends @a count @a items to the array @a a and returns the number
	/// of items in the array after the append operation.
	template <typename T> u32 push(Array<T>& a, const T* items, u32 count);

	/// Clears the content of the array @a a.
	/// @note
	/// Does not free memory nor call destructors, it only zeroes
	/// the number of items in the array.
	template <typename T> void clear(Array<T>& a);

	/// Returns a pointer to the first item in the array @a a.
	template <typename T> T* begin(Array<T>& a);

	/// Returns a pointer to the first item in the array @a a.
	template <typename T> const T* begin(const Array<T>& a);

	/// Returns a pointer to the item following the last item in the array @a a.
	template <typename T> T* end(Array<T>& a);

	/// Returns a pointer to the item following the last item in the array @a a.
	template <typename T> const T* end(const Array<T>& a);

	/// Returns the first element of the array @a a.
	template <typename T> T& front(Array<T>& a);

	/// Returns the first element of the array @a a.
	template <typename T> const T& front(const Array<T>& a);

	/// Returns the last element of the array @a a.
	template <typename T> T& back(Array<T>& a);

	/// Returns the last element of the array @a a.
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
	inline u32 size(const Array<T>& a)
	{
		return a._size;
	}

	template <typename T>
	inline u32 capacity(const Array<T>& a)
	{
		return a._capacity;
	}

	template <typename T>
	inline void resize(Array<T>& a, u32 size)
	{
		if (size > a._capacity)
			set_capacity(a, size);

		a._size = size;
	}

	template <typename T>
	inline void reserve(Array<T>& a, u32 capacity)
	{
		if (capacity > a._capacity)
			grow(a, capacity);
	}

	template <typename T>
	inline void set_capacity(Array<T>& a, u32 capacity)
	{
		if (capacity == a._capacity)
			return;

		if (capacity < a._size)
			resize(a, capacity);

		if (capacity > 0)
		{
			T* tmp = a._data;
			a._capacity = capacity;
			a._data = (T*)a._allocator->allocate(capacity * sizeof(T), alignof(T));

			memcpy(a._data, tmp, a._size * sizeof(T));

			a._allocator->deallocate(tmp);
		}
	}

	template <typename T>
	inline void grow(Array<T>& a, u32 min_capacity)
	{
		u32 new_capacity = a._capacity * 2 + 1;

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
	inline u32 push_back(Array<T>& a, const T& item)
	{
		if (a._capacity == a._size)
			grow(a, 0);

		memcpy(&a._data[a._size], &item, sizeof(T));

		return a._size++;
	}

	template <typename T>
	inline void pop_back(Array<T>& a)
	{
		CE_ASSERT(a._size > 0, "The array is empty");
		--a._size;
	}

	template <typename T>
	inline u32 push(Array<T>& a, const T* items, u32 count)
	{
		if (a._capacity <= a._size + count)
			grow(a, a._size + count);

		memcpy(&a._data[a._size], items, sizeof(T) * count);
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
		return a._data;
	}

	template <typename T>
	inline T* begin(Array<T>& a)
	{
		return a._data;
	}

	template <typename T>
	inline const T* end(const Array<T>& a)
	{
		return a._data + a._size;
	}

	template <typename T>
	inline T* end(Array<T>& a)
	{
		return a._data + a._size;
	}

	template <typename T>
	inline T& front(Array<T>& a)
	{
		CE_ASSERT(a._size > 0, "The array is empty");
		return a._data[0];
	}

	template <typename T>
	inline const T& front(const Array<T>& a)
	{
		CE_ASSERT(a._size > 0, "The array is empty");
		return a._data[0];
	}

	template <typename T>
	inline T& back(Array<T>& a)
	{
		CE_ASSERT(a._size > 0, "The array is empty");
		return a._data[a._size - 1];
	}

	template <typename T>
	inline const T& back(const Array<T>& a)
	{
		CE_ASSERT(a._size > 0, "The array is empty");
		return a._data[a._size - 1];
	}

} // namespace array

template <typename T>
inline Array<T>::Array(Allocator& a)
	: _allocator(&a)
	, _capacity(0)
	, _size(0)
	, _data(NULL)
{
}

template <typename T>
inline Array<T>::Array(const Array<T>& other)
	: _allocator(other._allocator)
	, _capacity(0)
	, _size(0)
	, _data(NULL)
{
	const u32 size = other._size;
	array::resize(*this, size);
	memcpy(_data, other._data, sizeof(T) * size);
}

template <typename T>
inline Array<T>::~Array()
{
	_allocator->deallocate(_data);
}

template <typename T>
inline T& Array<T>::operator[](u32 index)
{
	CE_ASSERT(index < _size, "Index out of bounds");
	return _data[index];
}

template <typename T>
inline const T& Array<T>::operator[](u32 index) const
{
	CE_ASSERT(index < _size, "Index out of bounds");
	return _data[index];
}

template <typename T>
inline Array<T>& Array<T>::operator=(const Array<T>& other)
{
	const u32 size = other._size;
	array::resize(*this, size);
	memcpy(_data, other._data, sizeof(T) * size);
	return *this;
}

} // namespace crown
