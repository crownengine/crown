/*
 * Copyright (c) 2012-2014 Daniele Bartolini and individual contributors.
 * License: https://github.com/taylor001/crown/blob/master/LICENSE
 */

#pragma once

#include "ceassert.h"
#include "types.h"
#include "container_types.h"

namespace crown
{

/// Packed array of objects with lookup table.
///
/// @ingroup Containers
template <uint32_t MAX, typename T>
struct IdArray
{
	/// Creates the table for tracking exactly @a MAX - 1 unique Ids.
	IdArray();

	/// Random access by index.
	T& operator[](uint32_t i);
	/// Random access by index.
	const T& operator[](uint32_t i) const;

	// The index of the first unused id
	uint16_t _freelist;

	// Next available unique id
	uint16_t _next_id;
	uint16_t _size;

	// The last valid id is reserved and cannot be used to
	// refer to Ids from the outside
	Id _sparse[MAX];
	uint16_t _sparse_to_dense[MAX];
	uint16_t _dense_to_sparse[MAX];
	T _objects[MAX];
};

/// Functions to manipulate IdArray.
///
/// @ingroup Containers
namespace id_array
{
	/// Creates a new @a object in the array @a a and returns its id.
	template <uint32_t MAX, typename T> Id create(IdArray<MAX, T>& a, const T& object);

	/// Destroys the object with the given @a id.
	template <uint32_t MAX, typename T> void destroy(IdArray<MAX, T>& a, Id id);

	/// Returns whether the table has the object with the given @a id.
	template <uint32_t MAX, typename T> bool has(const IdArray<MAX, T>& a, Id id);

	/// Returns the number of objects in the array.
	template <uint32_t MAX, typename T> uint32_t size(const IdArray<MAX, T>& a);

	/// Returns the object with the given @a id.
	template <uint32_t MAX, typename T> T& get(const Id& id);

	template <uint32_t MAX, typename T> T* begin(IdArray<MAX, T>& a);
	template <uint32_t MAX, typename T> const T* begin(const IdArray<MAX, T>& a);
	template <uint32_t MAX, typename T> T* end(IdArray<MAX, T>& a);
	template <uint32_t MAX, typename T> const T* end(const IdArray<MAX, T>& a);
} // namespace id_array

namespace id_array
{
	template <uint32_t MAX, typename T>
	inline Id create(IdArray<MAX, T>& a, const T& object)
	{
		CE_ASSERT(a._size < MAX, "Object list full");

		// Obtain a new id
		Id id;
		id.id = a._next_id++;

		// Recycle slot if there are any
		if (a._freelist != INVALID_ID)
		{
			id.index = a._freelist;
			a._freelist = a._sparse[a._freelist].index;
		}
		else
		{
			id.index = a._size;
		}

		a._sparse[id.index] = id;
		a._sparse_to_dense[id.index] = a._size;
		a._dense_to_sparse[a._size] = id.index;
		a._objects[a._size] = object;
		a._size++;

		return id;
	}

	template <uint32_t MAX, typename T>
	inline void destroy(IdArray<MAX, T>& a, Id id)
	{
		CE_ASSERT(has(a, id), "IdArray does not have ID: %d,%d", id.id, id.index);

		a._sparse[id.index].id = INVALID_ID;
		a._sparse[id.index].index = a._freelist;
		a._freelist = id.index;

		// Swap with last element
		const uint32_t last = a._size - 1;
		CE_ASSERT(last >= a._sparse_to_dense[id.index], "Swapping with previous item");
		a._objects[a._sparse_to_dense[id.index]] = a._objects[last];

		// Update tables
		uint16_t std = a._sparse_to_dense[id.index];
		uint16_t dts = a._dense_to_sparse[last];
		a._sparse_to_dense[dts] = std;
		a._dense_to_sparse[std] = dts;
		a._size--;
	}

	template <uint32_t MAX, typename T>
	inline T& get(IdArray<MAX, T>& a, const Id& id)
	{
		CE_ASSERT(has(a, id), "IdArray does not have ID: %d,%d", id.id, id.index);

		return a._objects[a._sparse_to_dense[id.index]];
	}

	template <uint32_t MAX, typename T>
	inline bool has(const IdArray<MAX, T>& a, Id id)
	{
		return id.index < MAX && a._sparse[id.index].id == id.id;
	}

	template <uint32_t MAX, typename T>
	inline uint32_t size(const IdArray<MAX, T>& a)
	{
		return a._size;
	}

	template <uint32_t MAX, typename T>
	inline T* begin(IdArray<MAX, T>& a)
	{
		return a._objects;
	}

	template <uint32_t MAX, typename T>
	inline const T* begin(const IdArray<MAX, T>& a)
	{
		return a._objects;
	}

	template <uint32_t MAX, typename T>
	inline T* end(IdArray<MAX, T>& a)
	{
		return a._objects + a._size;
	}

	template <uint32_t MAX, typename T>
	inline const T* end(const IdArray<MAX, T>& a)
	{
		return a._objects + a._size;
	}
} // namespace id_array

template <uint32_t MAX, typename T>
inline IdArray<MAX, T>::IdArray()
	: _freelist(INVALID_ID)
	, _next_id(0)
	, _size(0)
{
	for (uint32_t i = 0; i < MAX; i++)
	{
		_sparse[i].id = INVALID_ID;
	}
}

template <uint32_t MAX, typename T>
inline T& IdArray<MAX, T>::operator[](uint32_t i)
{
	CE_ASSERT(i < _size, "Index out of bounds");
	return _objects[i];
}

template <uint32_t MAX, typename T>
inline const T& IdArray<MAX, T>::operator[](uint32_t i) const
{
	CE_ASSERT(i < _size, "Index out of bounds");
	return _objects[i];
}

} // namespace crown
