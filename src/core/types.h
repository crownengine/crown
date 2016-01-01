/*
 * Copyright (c) 2012-2016 Daniele Bartolini and individual contributors.
 * License: https://github.com/taylor001/crown/blob/master/LICENSE
 */

#pragma once

#include <stddef.h> // NULL
#include <stdint.h>

namespace crown
{

#define INVALID_ID 65535

struct Id
{
	uint16_t id;
	uint16_t index;

	void decode(uint32_t id_and_index)
	{
		id = (id_and_index & 0xffff0000) >> 16;
		index = id_and_index & 0xffff;
	}

	uint32_t encode() const
	{
		return (uint32_t(id) << 16) | uint32_t(index);
	}

	bool operator==(const Id& other)
	{
		return id == other.id && index == other.index;
	}

	bool operator!=(const Id& other)
	{
		return id != other.id || index != other.index;
	}
};

} // namespace crown
