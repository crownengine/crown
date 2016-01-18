/*
 * Copyright (c) 2012-2016 Daniele Bartolini and individual contributors.
 * License: https://github.com/taylor001/crown/blob/master/LICENSE
 */

#pragma once

#include "types.h"
#include "container_types.h"
#include "resource_types.h"
#include "material.h"
#include "string_id.h"

namespace crown
{

class MaterialManager
{
	Allocator* _allocator;
	ResourceManager* _resource_manager;
	SortMap<StringId64, Material*> _materials;

public:

	MaterialManager(Allocator& a, ResourceManager& rm);
	~MaterialManager();

	/// Creates the material @a id.
	void create_material(StringId64 id);

	/// Destroys the material @a id.
	void destroy_material(StringId64 id);

	/// Returns the material @a id.
	Material* get(StringId64 id);
};

} // namespace crown
