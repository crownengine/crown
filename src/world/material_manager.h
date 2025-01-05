/*
 * Copyright (c) 2012-2025 Daniele Bartolini et al.
 * SPDX-License-Identifier: MIT
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
	Allocator *_allocator;
	ResourceManager *_resource_manager;
	HashMap<const MaterialResource *, Material *> _materials;

	///
	MaterialManager(Allocator &a, ResourceManager &rm);

	///
	~MaterialManager();

	///
	void online(StringId64 id, ResourceManager &rm);

	///
	void offline(StringId64 id, ResourceManager &rm);

	/// Instantiates the material @a resource.
	Material *create_material(const MaterialResource *resource);

	/// Returns the instance of the material @a resource.
	Material *get(const MaterialResource *resource);

	///
	void reload_textures(const TextureResource *old_resource, const TextureResource *new_resource);
};

} // namespace crown
