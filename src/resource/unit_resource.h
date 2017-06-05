/*
 * Copyright (c) 2012-2017 Daniele Bartolini and individual contributors.
 * License: https://github.com/taylor001/crown/blob/master/LICENSE
 */

#pragma once

#include "compiler_types.h"
#include "filesystem_types.h"
#include "memory_types.h"

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
	StringId32 type;
	u32 num_instances;
	u32 size;
//	u32 unit_index[num_instances]
//	char data[size]
};

namespace unit_resource_internal
{
	void compile(const char* path, CompileOptions& opts);

} // namespace unit_resource_internal

} // namespace crown
