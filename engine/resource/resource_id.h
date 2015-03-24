/*
 * Copyright (c) 2012-2015 Daniele Bartolini and individual contributors.
 * License: https://github.com/taylor001/crown/blob/master/LICENSE
 */

#pragma once

#include "string_utils.h"
#include "murmur.h"
#include "path.h"
#include <inttypes.h>

namespace crown
{

struct ResourceId
{
	ResourceId()
		: type(uint64_t(0))
		, name(uint64_t(0))
	{
	}

	ResourceId(StringId64 type, StringId64 name)
		: type(type)
		, name(name)
	{
	}

	ResourceId(const char* type, const char* name)
		: type(murmur64(type, strlen(type), 0))
		, name(murmur64(name, strlen(name), SEED))
	{
	}

	const char* to_string(char out[64])
	{
		snprintf(out, 64, "%.16" PRIx64 "-%.16" PRIx64, type.id(), name.id());
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

	StringId64 type;
	StringId64 name;
};

} // namespace crown
