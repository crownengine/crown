/*
 * Copyright (c) 2012-2020 Daniele Bartolini and individual contributors.
 * License: https://github.com/dbartolini/crown/blob/master/LICENSE
 */

#pragma once

#include "core/filesystem/types.h"
#include "core/memory/types.h"
#include "core/strings/string_id.h"
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
	u32 data_size;
//	u32 unit_index[num_instances]
//	Padding to 16-bytes boundary
//	char data[size]
};

namespace unit_resource_internal
{
	s32 compile(CompileOptions& opts);

} // namespace unit_resource_internal

namespace unit_resource
{
	/// Returns the first component data in the unit resource @ur.
	const ComponentData* component_data(const UnitResource* ur);

} // namespace unit_resource

} // namespace crown
