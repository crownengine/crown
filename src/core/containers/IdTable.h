/*
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

struct Id
{
	union
	{
		uint16_t id;
		uint16_t next;
	};

	uint16_t index;
};

/// Table of Ids.
class IdTable
{
public:

	/// Creates the table for tracking exactly @max_ids - 1 unique Ids.
					IdTable(Allocator& allocator, uint16_t max_ids);
					~IdTable();

	/// Returns a new Id.
	Id				create();

	/// Destroys the specified @id.
	void			destroy(Id id);

	/// Returns whether the table has the specified @id
	bool			has(Id id) const;

private:

	// Returns the next available unique id.
	uint16_t		next_id();

private:

	Allocator&		m_allocator;

	// The maximum number of Ids the table can track.
	// The last valid id is reserved and cannot be used to
	// refer to Ids from the outside.
	const uint16_t	m_max_ids;

	// The index of the first unused id.
	uint16_t		m_freelist;

	// The index of the last id in the id table.
	uint16_t		m_last_index;

	// Next available unique id.
	uint16_t		m_next_id;

	// Table of ids.
	Id*				m_ids;
};

//-----------------------------------------------------------------------------
inline IdTable::IdTable(Allocator& allocator, uint16_t max_ids) :
	m_allocator(allocator),
	m_max_ids(max_ids),
	m_freelist(max_ids),
	m_last_index(0),
	m_next_id(0)
{
	m_ids = (Id*)m_allocator.allocate(m_max_ids * sizeof(Id));
}

//-----------------------------------------------------------------------------
inline IdTable::~IdTable()
{
	if (m_ids)
	{
		m_allocator.deallocate(m_ids);
	}
}

//-----------------------------------------------------------------------------
inline Id IdTable::create()
{
	// Obtain a new id
	Id id;
	id.id = next_id();

	// Recycle slot if there are any
	if (m_freelist != m_max_ids)
	{
		id.index = m_freelist;
		m_freelist = m_ids[m_freelist].next;
	}
	else
	{
		id.index = m_last_index++;
	}

	m_ids[id.index] = id;

	return id;
}

//-----------------------------------------------------------------------------
inline void IdTable::destroy(Id id)
{
	CE_ASSERT(has(id), "IdTable does not have ID: %d,%d", id.id, id.index);

	m_ids[id.index].next = m_freelist;
	m_freelist = id.index;
}

//-----------------------------------------------------------------------------
inline bool IdTable::has(Id id) const
{
	return id.index < m_max_ids && m_ids[id.index].id == id.id;
}

//-----------------------------------------------------------------------------
inline uint16_t IdTable::next_id()
{
	CE_ASSERT(m_next_id < m_max_ids, "Maximum number of IDs reached");

	return m_next_id++;
}

} // namespace crown
