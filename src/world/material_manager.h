/*
 * Copyright (c) 2012-2016 Daniele Bartolini and individual contributors.
 * License: https://github.com/taylor001/crown/blob/master/LICENSE
 */

#pragma once

#include "container_types.h"
#include "filesystem_types.h"
#include "material.h"
#include "resource_types.h"
#include "string_id.h"
#include "types.h"

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

	void* load(File& file, Allocator& a);
	void online(StringId64 id, ResourceManager& rm);
	void offline(StringId64 id, ResourceManager& rm);
	void unload(Allocator& a, void* res);

	/// Creates the material @a id.
	void create_material(StringId64 id);

	/// Destroys the material @a id.
	void destroy_material(StringId64 id);

	/// Returns the material @a id.
	Material* get(StringId64 id);
};

} // namespace crown
