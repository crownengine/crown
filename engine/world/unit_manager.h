/*
 * Copyright (c) 2012-2015 Daniele Bartolini and individual contributors.
 * License: https://github.com/taylor001/crown/blob/master/LICENSE
 */

#include "types.h"
#include "array.h"

namespace crown
{

#define UNIT_INDEX_BITS 22
#define UNIT_INDEX_MASK 0x003fffff
#define UNIT_ID_BITS    8
#define UNIT_ID_MASK    0x3fc00000

struct UnitId
{
	uint32_t id;

	uint32_t index()
	{
		return id & UNIT_INDEX_MASK;
	}

	uint32_t id()
	{
		return (id >> UNIT_INDEX_BITS) & UNIT_ID_MASK;
	}
};

struct UnitManager
{
	Array<uint8_t> identifiers;

	UnitId create()
	{
	}

	void destroy(UnitId id)
	{

	}

	bool has(UnitId id)
	{
		return identifiers[id.index()] == id.id();
	}
};

} // namespace crown
