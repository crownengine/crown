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

namespace crown
{

/// Table of Ids.
template <uint32_t MAX_NUM_ID>
class IdTable
{
public:

	/// Creates the table for tracking exactly @a MAX_NUM_ID - 1 unique Ids.
					IdTable();

	/// Returns a new Id.
	Id				create();

	/// Destroys the specified @a id.
	void			destroy(Id id);

	/// Returns whether the table has the specified @a id
	bool			has(Id id) const;

	uint16_t		size() const;

	const Id*		begin() const;
	const Id*		end() const;

private:

	// Returns the next available unique id.
	uint16_t		next_id();

private:

	// The index of the first unused id.
	uint16_t		m_freelist;

	// The index of the last id in the id table.
	uint16_t		m_last_index;

	// Next available unique id.
	uint16_t		m_next_id;
	uint16_t		m_size;

	// Table of ids.
	// The last valid id is reserved and cannot be used to
	// refer to Ids from the outside.
	Id				m_ids[MAX_NUM_ID];
};

//-----------------------------------------------------------------------------
template <uint32_t MAX_NUM_ID>
inline IdTable<MAX_NUM_ID>::IdTable()
	: m_freelist(INVALID_ID)
	, m_last_index(0)
	, m_next_id(0)
	, m_size(0)
{
	for (uint32_t i = 0; i < MAX_NUM_ID; i++)
	{
		m_ids[i].id = INVALID_ID;
	}
}

//-----------------------------------------------------------------------------
template <uint32_t MAX_NUM_ID>
inline Id IdTable<MAX_NUM_ID>::create()
{
	// Obtain a new id
	Id id;
	id.id = next_id();

	// Recycle slot if there are any
	if (m_freelist != INVALID_ID)
	{
		id.index = m_freelist;
		m_freelist = m_ids[m_freelist].index;
	}
	else
	{
		id.index = m_last_index++;
	}

	m_ids[id.index] = id;

	m_size++;

	return id;
}

//-----------------------------------------------------------------------------
template <uint32_t MAX_NUM_ID>
inline void IdTable<MAX_NUM_ID>::destroy(Id id)
{
	CE_ASSERT(has(id), "IdTable does not have ID: %d,%d", id.id, id.index);

	m_ids[id.index].id = INVALID_ID;
	m_ids[id.index].index = m_freelist;
	m_freelist = id.index;
	m_size--;
}

//-----------------------------------------------------------------------------
template <uint32_t MAX_NUM_ID>
inline bool IdTable<MAX_NUM_ID>::has(Id id) const
{
	return id.index < MAX_NUM_ID && m_ids[id.index].id == id.id;
}

//-----------------------------------------------------------------------------
template <uint32_t MAX_NUM_ID>
inline uint16_t IdTable<MAX_NUM_ID>::size() const
{
	return m_size;
}

//-----------------------------------------------------------------------------
template <uint32_t MAX_NUM_ID>
inline const Id* IdTable<MAX_NUM_ID>::begin() const
{
	return m_ids;
}

//-----------------------------------------------------------------------------
template <uint32_t MAX_NUM_ID>
inline const Id* IdTable<MAX_NUM_ID>::end() const
{
	return m_ids + MAX_NUM_ID;
}

//-----------------------------------------------------------------------------
template <uint32_t MAX_NUM_ID>
inline uint16_t IdTable<MAX_NUM_ID>::next_id()
{
	CE_ASSERT(m_next_id < MAX_NUM_ID, "Maximum number of IDs reached");

	return m_next_id++;
}

} // namespace crown
