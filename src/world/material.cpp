/*
 * Copyright (c) 2012-2021 Daniele Bartolini et al.
 * License: https://github.com/dbartolini/crown/blob/master/LICENSE
 */

#include "resource/material_resource.h"
#include "resource/resource_manager.h"
#include "resource/texture_resource.h"
#include "world/material.h"
#include "world/shader_manager.h"
#include <bgfx/bgfx.h>

namespace crown
{
void Material::bind(ResourceManager& rm, ShaderManager& sm, u8 view, s32 depth) const
{
	using namespace material_resource;

	// Set samplers
	for (u32 i = 0; i < _resource->num_textures; ++i)
	{
		const TextureData* td   = texture_data(_resource, i);
		const TextureHandle* th = texture_handle(_resource, i, _data);

		const TextureResource* teximg = (TextureResource*)rm.get(RESOURCE_TYPE_TEXTURE, td->id);

		bgfx::UniformHandle sampler;
		bgfx::TextureHandle texture;
		sampler.idx = th->sampler_handle;
		texture.idx = teximg->handle.idx;

		bgfx::setTexture(i
			, sampler
			, texture
			, sm.sampler_state(_resource->shader, td->name)
			);
	}

	// Set uniforms
	for (u32 i = 0; i < _resource->num_uniforms; ++i)
	{
		const UniformHandle* uh = uniform_handle(_resource, i, _data);

		bgfx::UniformHandle buh;
		buh.idx = uh->uniform_handle;
		bgfx::setUniform(buh, (char*)uh + sizeof(uh->uniform_handle));
	}

	sm.submit(_resource->shader, view, depth);
}

void Material::set_float(StringId32 name, f32 value)
{
	char* p = (char*)material_resource::uniform_handle_by_name(_resource, name, _data);
	*(f32*)(p + sizeof(u32)) = value;
}

void Material::set_vector2(StringId32 name, const Vector2& value)
{
	char* p = (char*)material_resource::uniform_handle_by_name(_resource, name, _data);
	*(Vector2*)(p + sizeof(u32)) = value;
}

void Material::set_vector3(StringId32 name, const Vector3& value)
{
	char* p = (char*)material_resource::uniform_handle_by_name(_resource, name, _data);
	*(Vector3*)(p + sizeof(u32)) = value;
}

void Material::set_vector4(StringId32 name, const Vector4& value)
{
	char* p = (char*)material_resource::uniform_handle_by_name(_resource, name, _data);
	*(Vector4*)(p + sizeof(u32)) = value;
}

void Material::set_matrix4x4(StringId32 name, const Matrix4x4& value)
{
	char* p = (char*)material_resource::uniform_handle_by_name(_resource, name, _data);
	*(Matrix4x4*)(p + sizeof(u32)) = value;
}

} // namespace crown
