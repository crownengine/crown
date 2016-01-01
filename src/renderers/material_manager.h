/*
 * Copyright (c) 2012-2016 Daniele Bartolini and individual contributors.
 * License: https://github.com/taylor001/crown/blob/master/LICENSE
 */

#pragma once

#include "types.h"
#include "container_types.h"
#include "material.h"
#include "resource_manager.h"
#include "resource_types.h"
#include "id_table.h"
#include <bgfx/bgfx.h>

namespace crown
{

typedef Id MaterialId;

struct MaterialManager
{
	MaterialManager();

	MaterialId create_material(StringId64 id);
	void destroy_material(MaterialId id);
	Material* lookup_material(MaterialId id);

private:

	IdTable<512> _materials_ids;
	Material _materials[512];
};

namespace material_manager
{
	void init();
	void shutdown();
	MaterialManager* get();

} // namespace material_manager

} // namespace crown
