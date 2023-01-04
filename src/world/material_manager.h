/*
 * Copyright (c) 2012-2023 Daniele Bartolini et al.
 * SPDX-License-Identifier: MIT
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
	HashMap<const MaterialResource *, Material *> _materials;

	///
	MaterialManager(Allocator &a);

	///
	~MaterialManager();

	///
	void *load(File &file, Allocator &a);

	///
	void online(StringId64 id, ResourceManager &rm);

	///
	void offline(StringId64 id, ResourceManager &rm);

	///
	void unload(Allocator &a, void *res);

	/// Instantiates the material @a resource.
	Material *create_material(const MaterialResource *resource);

	/// Destroys the instance of the material @a resource.
	void destroy_material(const MaterialResource *resource);

	/// Returns the instance of the material @a resource.
	Material *get(const MaterialResource *resource);
};

} // namespace crown
