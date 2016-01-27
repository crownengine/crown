/*
 * Copyright (c) 2012-2016 Daniele Bartolini and individual contributors.
 * License: https://github.com/taylor001/crown/blob/master/LICENSE
 */

#pragma once

#include "memory_types.h"
#include "filesystem_types.h"
#include "compiler_types.h"

namespace crown
{
struct UnitResource
{
	u32 version;
	u32 num_units;
	u32 num_component_types;
//	ComponentData data[num_component_types]
};

struct ComponentData
{
	u32 type;
	u32 num_instances;
	u32 size;
//	u32 unit_index[num_instances]
//	char data[size]
};

namespace unit_resource
{
	void compile(const char* path, CompileOptions& opts);
	void* load(File& file, Allocator& a);
	void unload(Allocator& allocator, void* resource);
} // namespace unit_resource

} // namespace crown
