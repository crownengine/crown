/*
 * Copyright (c) 2012-2015 Daniele Bartolini and individual contributors.
 * License: https://github.com/taylor001/crown/blob/master/LICENSE
 */

#pragma once

#include "memory_types.h"
#include "resource_types.h"
#include "filesystem_types.h"
#include "compiler_types.h"
#include "container_types.h"

namespace crown
{

struct PackageResource
{
	struct Resource
	{
		StringId64 type;
		StringId64 name;

		Resource(StringId64 t, StringId64 n)
			: type(t)
			, name(n)
		{
		}

		bool operator<(const Resource& r) const
		{
			return type < r.type;
		}

		bool operator==(const Resource& r) const
		{
			return type == r.type && name == r.name;
		}
	};

	PackageResource(Allocator& a)
		: resources(a)
	{
	}

	Array<Resource> resources;
};

namespace package_resource
{
	void compile(const char* path, CompileOptions& opts);
	void* load(File& file, Allocator& a);
	void online(StringId64 /*id*/, ResourceManager& /*rm*/);
	void offline(StringId64 /*id*/, ResourceManager& /*rm*/);
	void unload(Allocator& allocator, void* resource);
} // namespace package_resource
} // namespace crown
