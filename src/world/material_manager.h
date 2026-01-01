/*
 * Copyright (c) 2012-2026 Daniele Bartolini et al.
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
	ShaderManager *_shader_manager;
	HashMap<const MaterialResource *, Material *> _materials;

	///
	MaterialManager(Allocator &a, ResourceManager &rm, ShaderManager &sm);

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

	///
	void reload_shaders(const ShaderResource *old_resource, const ShaderResource *new_resource);
};

} // namespace crown
