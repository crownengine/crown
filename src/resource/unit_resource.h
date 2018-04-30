/*
 * Copyright (c) 2012-2018 Daniele Bartolini and individual contributors.
 * License: https://github.com/dbartolini/crown/blob/master/LICENSE
 */

#pragma once

#include "core/filesystem/types.h"
#include "core/memory/types.h"
#include "resource/types.h"

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
	void compile(CompileOptions& opts);

} // namespace unit_resource_internal

} // namespace crown
