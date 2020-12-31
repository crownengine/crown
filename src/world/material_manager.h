/*
 * Copyright (c) 2012-2021 Daniele Bartolini et al.
 * License: https://github.com/dbartolini/crown/blob/master/LICENSE
 */

#pragma once

#include "core/containers/types.h"
#include "core/filesystem/types.h"
#include "core/strings/string_id.h"
#include "core/types.h"
#include "resource/types.h"
#include "world/material.h"

namespace crown
{
struct MaterialManager
{
	Allocator* _allocator;
	ResourceManager* _resource_manager;
	HashMap<StringId64, Material*> _materials;

	///
	MaterialManager(Allocator& a, ResourceManager& rm);

	///
	~MaterialManager();

	///
	void* load(File& file, Allocator& a);

	///
	void online(StringId64 id, ResourceManager& rm);

	///
	void offline(StringId64 id, ResourceManager& rm);

	///
	void unload(Allocator& a, void* res);

	/// Creates the material @a id.
	void create_material(StringId64 id);

	/// Destroys the material @a id.
	void destroy_material(StringId64 id);

	/// Returns the material @a id.
	Material* get(StringId64 id);
};

} // namespace crown
