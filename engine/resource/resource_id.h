/*
 * Copyright (c) 2012-2014 Daniele Bartolini and individual contributors.
 * License: https://github.com/taylor001/crown/blob/master/LICENSE
 */

#pragma once

#include "string_utils.h"
#include "murmur.h"
#include <inttypes.h>

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
		: type(murmur64(type, strlen(type), SEED))
		, name(murmur64(name, strlen(name), SEED))
	{
	}

	ResourceId(uint64_t type, uint64_t name)
		: type(type)
		, name(name)
	{
	}

	const char* to_string(char out[64])
	{
		snprintf(out, 64, "%.16" PRIx64 "-%.16" PRIx64, type, name);
		return out;
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
