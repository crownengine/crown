/*
 * Copyright (c) 2012-2014 Daniele Bartolini and individual contributors.
 * License: https://github.com/taylor001/crown/blob/master/LICENSE
 */

#pragma once

#include "assert.h"
#include "allocator.h"
#include "types.h"

namespace crown
{

/// Table of Ids.
///
/// @ingroup Containers
template <uint32_t MAX>
struct IdTable
{
	/// Creates the table for tracking exactly @a MAX - 1 unique Ids.
	IdTable();

	// The index of the first unused id.
	uint16_t _freelist;

	// Next available unique id.
	uint16_t _next_id;
	uint16_t _size;

	// Table of ids.
	// The last valid id is reserved and cannot be used to
	// refer to Ids from the outside.
	Id _ids[MAX];
};

/// Functions to manipulate IdTable.
///
/// @ingroup Containers
namespace id_table
{
	/// Creates a new Id in the table @a a and returns its value.
	template <uint32_t MAX> Id create(IdTable<MAX>& a);

	/// Destroys the @a id in the table @a a.
	template <uint32_t MAX> void destroy(IdTable<MAX>& a, Id id);

	/// Returns whether the table @a a has the given @a id.
	template <uint32_t MAX> bool has(const IdTable<MAX>& a, Id id);

	/// Returns the number of ids in the table @a a.
	template <uint32_t MAX> uint16_t size(const IdTable<MAX>& a);

	template <uint32_t MAX> const Id* begin(const IdTable<MAX>& a);
	template <uint32_t MAX> const Id* end(const IdTable<MAX>& a);
} // namespace id_table

namespace id_table
{
	template <uint32_t MAX>
	inline Id create(IdTable<MAX>& a)
	{
		// Obtain a new id
		Id id;
		id.id = a._next_id++;

		// Recycle slot if there are any
		if (a._freelist != INVALID_ID)
		{
			id.index = a._freelist;
			a._freelist = a._ids[a._freelist].index;
		}
		else
		{
			id.index = a._size;
		}

		a._ids[id.index] = id;
		a._size++;

		return id;
	}

	template <uint32_t MAX>
	inline void destroy(IdTable<MAX>& a, Id id)
	{
		CE_ASSERT(has(a, id), "IdTable does not have ID: %d,%d", id.id, id.index);

		a._ids[id.index].id = INVALID_ID;
		a._ids[id.index].index = a._freelist;
		a._freelist = id.index;
		a._size--;
	}

	template <uint32_t MAX>
	inline bool has(const IdTable<MAX>& a, Id id)
	{
		return id.index < MAX && a._ids[id.index].id == id.id;
	}

	template <uint32_t MAX>
	inline uint16_t size(const IdTable<MAX>& a)
	{
		return a._size;
	}

	template <uint32_t MAX>
	inline const Id* begin(const IdTable<MAX>& a)
	{
		return a._ids;
	}

	template <uint32_t MAX>
	inline const Id* end(const IdTable<MAX>& a)
	{
		return a._ids + MAX;
	}
} // namespace id_table

template <uint32_t MAX>
inline IdTable<MAX>::IdTable()
	: _freelist(INVALID_ID)
	, _next_id(0)
	, _size(0)
{
	for (uint32_t i = 0; i < MAX; i++)
	{
		_ids[i].id = INVALID_ID;
	}
}

} // namespace crown
