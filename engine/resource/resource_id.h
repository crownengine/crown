/*
 * Copyright (c) 2012-2014 Daniele Bartolini and individual contributors.
 * License: https://github.com/taylor001/crown/blob/master/LICENSE
 */

#pragma once

#include "string_utils.h"

namespace crown
{

struct ResourceId
{
	ResourceId()
		: type(0)
		, name(0)
	{
	}

	ResourceId(const char* type, const char* name)
		: type(murmur2_64(type, strlen(type), SEED))
		, name(murmur2_64(name, strlen(name), SEED))
	{
	}

	ResourceId(uint64_t type, uint64_t name)
		: type(type)
		, name(name)
	{
	}

	bool operator==(const ResourceId& a) const
	{
		return type == a.type && name == a.name;
	}

	bool operator<(const ResourceId& a) const
	{
		return type < a.type || (type == a.type && name < a.name);
	}

	static const uint64_t SEED = 0;

	uint64_t type;
	uint64_t name;
};

} // namespace crown
