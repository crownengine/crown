/*
 * Copyright (c) 2012-2018 Daniele Bartolini and individual contributors.
 * License: https://github.com/dbartolini/crown/blob/master/LICENSE
 */

#pragma once

#include "core/containers/types.h"
#include "core/filesystem/types.h"
#include "core/memory/types.h"
#include "core/strings/string_id.h"
#include "core/strings/types.h"
#include "resource/types.h"
#include "resource/types.h"

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

namespace package_resource_internal
{
	void compile(CompileOptions& opts);
	void* load(File& file, Allocator& a);
	void unload(Allocator& allocator, void* resource);

} // namespace package_resource_internal

} // namespace crown
