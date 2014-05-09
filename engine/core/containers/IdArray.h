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

#include "Assert.h"
#include "Types.h"
#include "ContainerTypes.h"

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
	uint16_t m_freelist;

	// Next available unique id
	uint16_t m_next_id;
	uint16_t m_size;

	// The last valid id is reserved and cannot be used to
	// refer to Ids from the outside
	Id m_sparse[MAX];
	uint16_t m_sparse_to_dense[MAX];
	uint16_t m_dense_to_sparse[MAX];
	T m_objects[MAX];
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
	//-----------------------------------------------------------------------------
	template <uint32_t MAX, typename T>
	inline Id create(IdArray<MAX, T>& a, const T& object)
	{
		CE_ASSERT(a.m_size < MAX, "Object list full");

		// Obtain a new id
		Id id;
		id.id = a.m_next_id++;

		// Recycle slot if there are any
		if (a.m_freelist != INVALID_ID)
		{
			id.index = a.m_freelist;
			a.m_freelist = a.m_sparse[a.m_freelist].index;
		}
		else
		{
			id.index = a.m_size;
		}

		a.m_sparse[id.index] = id;
		a.m_sparse_to_dense[id.index] = a.m_size;
		a.m_dense_to_sparse[a.m_size] = id.index;
		a.m_objects[a.m_size] = object;
		a.m_size++;

		return id;
	}

	//-----------------------------------------------------------------------------
	template <uint32_t MAX, typename T>
	inline void destroy(IdArray<MAX, T>& a, Id id)
	{
		CE_ASSERT(has(a, id), "IdArray does not have ID: %d,%d", id.id, id.index);

		a.m_sparse[id.index].id = INVALID_ID;
		a.m_sparse[id.index].index = a.m_freelist;
		a.m_freelist = id.index;

		// Swap with last element
		const uint32_t last = a.m_size - 1;
		CE_ASSERT(last >= a.m_sparse_to_dense[id.index], "Swapping with previous item");
		a.m_objects[a.m_sparse_to_dense[id.index]] = a.m_objects[last];

		// Update tables
		uint16_t std = a.m_sparse_to_dense[id.index];
		uint16_t dts = a.m_dense_to_sparse[last];
		a.m_sparse_to_dense[dts] = std;
		a.m_dense_to_sparse[std] = dts;
		a.m_size--;
	}

	//-----------------------------------------------------------------------------
	template <uint32_t MAX, typename T>
	inline T& get(IdArray<MAX, T>& a, const Id& id)
	{
		CE_ASSERT(has(a, id), "IdArray does not have ID: %d,%d", id.id, id.index);

		return a.m_objects[a.m_sparse_to_dense[id.index]];
	}

	//-----------------------------------------------------------------------------
	template <uint32_t MAX, typename T>
	inline bool has(const IdArray<MAX, T>& a, Id id)
	{
		return id.index < MAX && a.m_sparse[id.index].id == id.id;
	}

	//-----------------------------------------------------------------------------
	template <uint32_t MAX, typename T>
	inline uint32_t size(const IdArray<MAX, T>& a)
	{
		return a.m_size;
	}

	//-----------------------------------------------------------------------------
	template <uint32_t MAX, typename T>
	inline T* begin(IdArray<MAX, T>& a)
	{
		return a.m_objects;
	}

	//-----------------------------------------------------------------------------
	template <uint32_t MAX, typename T>
	inline const T* begin(const IdArray<MAX, T>& a)
	{
		return a.m_objects;
	}

	//-----------------------------------------------------------------------------
	template <uint32_t MAX, typename T>
	inline T* end(IdArray<MAX, T>& a)
	{
		return a.m_objects + a.m_size;
	}

	//-----------------------------------------------------------------------------
	template <uint32_t MAX, typename T>
	inline const T* end(const IdArray<MAX, T>& a)
	{
		return a.m_objects + a.m_size;
	}
} // namespace id_array

//-----------------------------------------------------------------------------
template <uint32_t MAX, typename T>
inline IdArray<MAX, T>::IdArray()
	: m_freelist(INVALID_ID)
	, m_next_id(0)
	, m_size(0)
{
	for (uint32_t i = 0; i < MAX; i++)
	{
		m_sparse[i].id = INVALID_ID;
	}
}

//-----------------------------------------------------------------------------
template <uint32_t MAX, typename T>
inline T& IdArray<MAX, T>::operator[](uint32_t i)
{
	CE_ASSERT(i < m_size, "Index out of bounds");
	return m_objects[i];
}

//-----------------------------------------------------------------------------
template <uint32_t MAX, typename T>
inline const T& IdArray<MAX, T>::operator[](uint32_t i) const
{
	CE_ASSERT(i < m_size, "Index out of bounds");
	return m_objects[i];
}

} // namespace crown
