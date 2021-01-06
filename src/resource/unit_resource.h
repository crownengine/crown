/*
 * Copyright (c) 2012-2021 Daniele Bartolini et al.
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
//	u32 parents[num_units]
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
	/// Returns the array of parents in the unit resource @ur.
	const u32* parents(const UnitResource* ur);

	/// Returns the first component type data in the unit resource @a ur, or, if
	/// @a component is != NULL, it returns the next component type data after
	/// it.
	const ComponentData* component_type_data(const UnitResource* ur, const ComponentData* component);

	/// Returns the payload data (the actual component data) for the @a component.
	const char* component_payload(const ComponentData* component);

	/// Returns the unit index of the @a component.
	const u32* component_unit_index(const ComponentData* component);

} // namespace unit_resource

} // namespace crown
