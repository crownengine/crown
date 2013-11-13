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
#include "Allocator.h"
#include "Types.h"
#include "List.h"

namespace crown
{

/// Table of Ids.
template <uint32_t MAX_NUM_ID, typename T>
class IdArray
{
public:

	/// Creates the table for tracking exactly @a MAX_NUM_ID - 1 unique Ids.
					IdArray(Allocator& a);

	/// Random access by Id
	T&				operator[](const Id& id);
	const T&		operator[](const Id& id) const;

	/// Returns a new Id.
	Id				create(const T& object);

	/// Destroys the specified @a id.
	void			destroy(Id id);

	/// Returns whether the table has the specified @a id
	bool			has(Id id) const;

	T&				lookup(const Id& id);

private:

	// Returns the next available unique id.
	uint16_t		next_id();

public:

	// The index of the first unused id
	uint16_t		m_freelist;

	// The index of the last id in the id table
	uint16_t		m_last_index;

	// Next available unique id
	uint16_t		m_next_id;


	// The last valid id is reserved and cannot be used to
	// refer to Ids from the outside
	Id				m_sparse[MAX_NUM_ID];
	uint16_t		m_sparse_to_dense[MAX_NUM_ID];
	uint16_t		m_dense_to_sparse[MAX_NUM_ID];
	List<T>			m_objects;
};

//-----------------------------------------------------------------------------
template <uint32_t MAX_NUM_ID, typename T>
inline IdArray<MAX_NUM_ID, T>::IdArray(Allocator& a)
	: m_freelist(MAX_NUM_ID)
	, m_last_index(0)
	, m_next_id(0)
	, m_objects(a)
{
	for (uint32_t i = 0; i < MAX_NUM_ID; i++)
	{
		m_sparse[i].id = INVALID_ID;
	}
}

//-----------------------------------------------------------------------------
template <uint32_t MAX_NUM_ID, typename T>
inline T& IdArray<MAX_NUM_ID, T>::operator[](const Id& id)
{
	return lookup(id);
}

//-----------------------------------------------------------------------------
template <uint32_t MAX_NUM_ID, typename T>
inline const T& IdArray<MAX_NUM_ID, T>::operator[](const Id& id) const
{
	return lookup(id);
}

//-----------------------------------------------------------------------------
template <uint32_t MAX_NUM_ID, typename T>
inline Id IdArray<MAX_NUM_ID, T>::create(const T& object)
{
	// Obtain a new id
	Id id;
	id.id = next_id();

	uint16_t dense_index;

	// Recycle slot if there are any
	if (m_freelist != MAX_NUM_ID)
	{
		id.index = m_freelist;
		m_freelist = m_sparse[m_freelist].id;
		m_objects[id.index] = object;
		dense_index = id.index;
	}
	else
	{
		id.index = m_last_index++;
		dense_index = m_objects.push_back(object);
	}

	m_sparse[id.index] = id;
	m_sparse_to_dense[id.index] = dense_index;
	m_dense_to_sparse[dense_index] = id.index;

	return id;
}

//-----------------------------------------------------------------------------
template <uint32_t MAX_NUM_ID, typename T>
inline void IdArray<MAX_NUM_ID, T>::destroy(Id id)
{
	CE_ASSERT(has(id), "IdArray does not have ID: %d,%d", id.id, id.index);

	m_sparse[id.index].id = INVALID_ID;
	m_sparse[id.index].index = m_freelist;
	m_freelist = id.index;

	// Swap with last element
	m_objects[m_sparse_to_dense[id.index]] = m_objects.back();
	m_objects.pop_back();

	// Update conversion tables
	//m_sparse_to_dense[m_dense_to_sparse[m_dense.size() - 1]] = id.index;
}

//-----------------------------------------------------------------------------
template <uint32_t MAX_NUM_ID, typename T>
inline T& IdArray<MAX_NUM_ID, T>::lookup(const Id& id)
{
	CE_ASSERT(has(id), "IdArray does not have ID: %d,%d", id.id, id.index);

	return m_objects[m_sparse_to_dense[id.index]];
}

//-----------------------------------------------------------------------------
template <uint32_t MAX_NUM_ID, typename T>
inline bool IdArray<MAX_NUM_ID, T>::has(Id id) const
{
	return id.index < MAX_NUM_ID && m_sparse[id.index].id == id.id;
}

//-----------------------------------------------------------------------------
template <uint32_t MAX_NUM_ID, typename T>
inline uint16_t IdArray<MAX_NUM_ID, T>::next_id()
{
	CE_ASSERT(m_next_id < MAX_NUM_ID, "Maximum number of IDs reached");

	return m_next_id++;
}

} // namespace crown
