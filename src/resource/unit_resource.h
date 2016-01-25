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
	uint32_t version;
	uint32_t num_units;
	uint32_t num_component_types;
//	ComponentData data[num_component_types]
};

struct ComponentData
{
	uint32_t type;
	uint32_t num_instances;
	uint32_t size;
//	uint32_t unit_index[num_instances]
//	char data[size]
};

namespace unit_resource
{
	void compile(const char* path, CompileOptions& opts);
	void* load(File& file, Allocator& a);
	void unload(Allocator& allocator, void* resource);
} // namespace unit_resource

} // namespace crown
