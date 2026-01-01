/*
 * Copyright (c) 2012-2026 Daniele Bartolini et al.
 * SPDX-License-Identifier: MIT
 */

#include "core/containers/array.inl"
#include "core/strings/string_id.inl"
#include "resource/material_resource.h"
#include "resource/resource_manager.h"
#include "resource/texture_resource.h"
#include "world/material.h"
#include "world/shader_manager.h"
#include <bgfx/bgfx.h>

namespace crown
{
Material::Material(Allocator &a)
#if CROWN_CAN_RELOAD
	: _texture_resources(a)
#endif
{
	CE_UNUSED(a);
}

void Material::bind(u8 view, u32 depth) const
{
	using namespace material_resource;

	// Set samplers.
	const TextureData *td = texture_data_array(_resource);
	for (u32 i = 0; i < _resource->num_textures; ++i) {
		const TextureHandle *th = texture_handle(td, i, _data);

		bgfx::UniformHandle sampler;
		bgfx::TextureHandle texture;
		sampler.idx = th->sampler_handle;
		texture.idx = th->texture_handle;
		u32 sampler_state = 0u;

		for (u32 i = 0; i < countof(_shader.samplers); ++i) {
			if (_shader.samplers[i].name == td->name._id)
				sampler_state = _shader.samplers[i].state;
		}

		bgfx::setTexture(i, sampler, texture, sampler_state);
	}

	// Set uniforms.
	const UniformData *ud = uniform_data_array(_resource);
	for (u32 i = 0; i < _resource->num_uniforms; ++i) {
		const UniformHandle *uh = uniform_handle(ud, i, _data);

		bgfx::UniformHandle buh;
		buh.idx = uh->uniform_handle;
		bgfx::setUniform(buh, (char *)uh + sizeof(uh->uniform_handle));
	}

	bgfx::setState(_shader.state | BGFX_STATE_MSAA);
	bgfx::submit(view, _shader.program, depth);
}

template<typename T>
static void set_uniform_value(StringId32 name, const T &value, const MaterialResource *mr, char *dynamic_data)
{
	UniformData *ud = material_resource::uniform_data_array(mr);
	u32 idx = material_resource::uniform_data_index(mr, ud, name);
	char *p = (char *)material_resource::uniform_handle(ud, idx, dynamic_data);
	*(T *)(p + sizeof(u32)) = value;
}

void Material::set_float(StringId32 name, f32 value)
{
	set_uniform_value(name, value, _resource, _data);
}

void Material::set_vector2(StringId32 name, const Vector2 &value)
{
	set_uniform_value(name, value, _resource, _data);
}

void Material::set_vector3(StringId32 name, const Vector3 &value)
{
	set_uniform_value(name, value, _resource, _data);
}

void Material::set_vector4(StringId32 name, const Vector4 &value)
{
	set_uniform_value(name, value, _resource, _data);
}

void Material::set_matrix4x4(StringId32 name, const Matrix4x4 &value)
{
	set_uniform_value(name, value, _resource, _data);
}

void Material::set_texture(StringId32 sampler_name, ResourceId texture_resource)
{
	const TextureResource *tr = (TextureResource *)_resource_manager->get(RESOURCE_TYPE_TEXTURE, texture_resource);
	TextureData *td = material_resource::texture_data_array(_resource);
	u32 idx = material_resource::texture_data_index(_resource, td, sampler_name);
	TextureHandle *th = (TextureHandle *)material_resource::texture_handle(td, idx, _data);
	th->texture_handle = tr->handle.idx;
#if CROWN_CAN_RELOAD
	u32 index = material_resource::texture_data_index(_resource, td, sampler_name);
	_texture_resources[index] = (TextureResource *)tr;
#endif
}

void Material::reload_textures(const TextureResource *old_resource, const TextureResource *new_resource)
{
#if CROWN_CAN_RELOAD
	const TextureData *td = material_resource::texture_data_array(_resource);
	for (u32 i = 0; i < array::size(_texture_resources); ++i) {
		if (_texture_resources[i] == old_resource) {
			_texture_resources[i] = (TextureResource *)new_resource;
			TextureHandle *th = material_resource::texture_handle(td, i, _data);
			th->texture_handle = new_resource->handle.idx;
		}
	}
#else
	CE_UNUSED_2(old_resource, new_resource);
#endif
}

} // namespace crown
