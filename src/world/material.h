/*
 * Copyright (c) 2012-2026 Daniele Bartolini et al.
 * SPDX-License-Identifier: MIT
 */

#pragma once

#include "config.h"
#include "core/math/types.h"
#include "resource/resource_id.h"
#include "resource/types.h"
#include "resource/shader_resource.h"
#include "world/types.h"

namespace crown
{
/// Material
///
/// @ingroup World
struct Material
{
	ResourceManager *_resource_manager;
	const MaterialResource *_resource;
	char *_data;
	ShaderData _shader;
#if CROWN_CAN_RELOAD
	Array<TextureResource *> _texture_resources;
#endif

	///
	explicit Material(Allocator &a);

	///
	void bind(u8 view, u32 depth = 0u) const;

	/// Sets the @a value of the variable @a name.
	void set_float(StringId32 name, f32 value);

	/// Sets the @a value of the variable @a name.
	void set_vector2(StringId32 name, const Vector2 &value);

	/// Sets the @a value of the variable @a name.
	void set_vector3(StringId32 name, const Vector3 &value);

	/// Sets the @a value of the variable @a name.
	void set_vector4(StringId32 name, const Vector4 &value);

	/// Sets the @a value of the variable @a name.
	void set_matrix4x4(StringId32 name, const Matrix4x4 &value);

	/// Sets the @a texture_resource of the sampler @a sampler_name.
	void set_texture(StringId32 sampler_name, ResourceId texture_resource);

	///
	void reload_textures(const TextureResource *old_resource, const TextureResource *new_resource);
};

} // namespace crown
