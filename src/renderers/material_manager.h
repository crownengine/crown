/*
 * Copyright (c) 2012-2015 Daniele Bartolini and individual contributors.
 * License: https://github.com/taylor001/crown/blob/master/LICENSE
 */

#pragma once

#include "types.h"
#include "container_types.h"
#include "resource_types.h"

namespace crown
{

struct MaterialManager
{
	MaterialManager(ResourceManager& rm);
	~MaterialManager();

	void create_material(StringId64 id);
	void destroy_material(StringId64 id);
	Material* lookup_material(StringId64 id);

private:

	ResourceManager* _resource_manager;
	SortMap<StringId64, Material*> _materials;
};

namespace material_manager
{
	void init(ResourceManager& rm);
	void shutdown();
	MaterialManager* get();
} // namespace material_manager

} // namespace crown
