/*
 * Copyright (c) 2012-2026 Daniele Bartolini et al.
 * SPDX-License-Identifier: MIT
 */

#include "core/containers/array.inl"
#include "core/containers/hash_map.inl"
#include "core/filesystem/file.h"
#include "core/strings/string_id.inl"
#include "resource/material_resource.h"
#include "resource/resource_manager.h"
#include "resource/texture_resource.h"
#include "world/material_manager.h"
#include "world/shader_manager.h"
#include <bgfx/bgfx.h>
#include <string.h> // memcpy

namespace crown
{
static const bgfx::UniformType::Enum s_bgfx_uniform_type[] =
{
	bgfx::UniformType::Vec4, // UniformType::FLOAT
	bgfx::UniformType::Vec4, // UniformType::VECTOR2
	bgfx::UniformType::Vec4, // UniformType::VECTOR3
	bgfx::UniformType::Vec4, // UniformType::VECTOR4
	bgfx::UniformType::Mat4  // UniformType::MATRIX4X4
};
CE_STATIC_ASSERT(countof(s_bgfx_uniform_type) == UniformType::COUNT);

MaterialManager::MaterialManager(Allocator &a, ResourceManager &rm, ShaderManager &sm)
	: _allocator(&a)
	, _resource_manager(&rm)
	, _shader_manager(&sm)
	, _materials(a)
{
}

MaterialManager::~MaterialManager()
{
	auto cur = hash_map::begin(_materials);
	auto end = hash_map::end(_materials);
	for (; cur != end; ++cur) {
		HASH_MAP_SKIP_HOLE(_materials, cur);

		_allocator->deallocate(cur->second);
	}
}

void MaterialManager::online(StringId64 id, ResourceManager &rm)
{
	using namespace material_resource;

	MaterialResource *mr = (MaterialResource *)rm.get(RESOURCE_TYPE_MATERIAL, id);
	Material *material = create_material(mr);

	const TextureData *td = texture_data_array(mr);
	for (u32 i = 0; i < mr->num_textures; ++i) {
		TextureHandle *th   = texture_handle(td, i, material->_data);
		TextureResource *tr = (TextureResource *)rm.get(RESOURCE_TYPE_TEXTURE, td[i].id);
		th->sampler_handle = bgfx::createUniform(texture_name(mr, td, i), bgfx::UniformType::Sampler).idx;
		th->texture_handle = tr->handle.idx;
#if CROWN_CAN_RELOAD
		material->_texture_resources[i] = tr;
#endif
	}

	const UniformData *ud = uniform_data_array(mr);
	for (u32 i = 0; i < mr->num_uniforms; ++i) {
		UniformHandle *uh  = uniform_handle(ud, i, material->_data);
		uh->uniform_handle = bgfx::createUniform(uniform_name(mr, ud, i), s_bgfx_uniform_type[ud[i].type]).idx;
	}
}

void MaterialManager::offline(StringId64 id, ResourceManager &rm)
{
	using namespace material_resource;

	const MaterialResource *mr = (MaterialResource *)rm.get(RESOURCE_TYPE_MATERIAL, id);
	Material *material = hash_map::get(_materials, mr, (Material *)NULL);

	const TextureData *td = texture_data_array(mr);
	for (u32 i = 0; i < mr->num_textures; ++i) {
		TextureHandle *th = texture_handle(td, i, material->_data);
		bgfx::UniformHandle sh;
		sh.idx = th->sampler_handle;
		bgfx::destroy(sh);
	}

	const UniformData *ud = uniform_data_array(mr);
	for (u32 i = 0; i < mr->num_uniforms; ++i) {
		UniformHandle *uh = uniform_handle(ud, i, material->_data);
		bgfx::UniformHandle bgfx_uh;
		bgfx_uh.idx = uh->uniform_handle;
		bgfx::destroy(bgfx_uh);
	}

	CE_DELETE(*_allocator, material);
	hash_map::remove(_materials, mr);
}

Material *MaterialManager::create_material(const MaterialResource *resource)
{
	Material *material = hash_map::get(_materials, resource, (Material *)NULL);
	if (material != NULL)
		return material;

	const u32 size = sizeof(Material) + resource->dynamic_data_size;
	material = (Material *)_allocator->allocate(size);
	new (material) Material(*_allocator);
	material->_resource_manager = _resource_manager;
	material->_resource = resource;
	material->_data = (char *)&material[1];
	material->_shader = _shader_manager->shader(resource->shader);

	const char *dynamic_data = (char *)resource + resource->dynamic_data_offset;
	memcpy(material->_data, dynamic_data, resource->dynamic_data_size);

#if CROWN_CAN_RELOAD
	array::resize(material->_texture_resources, resource->num_textures);
#endif

	hash_map::set(_materials, resource, material);
	return material;
}

Material *MaterialManager::get(const MaterialResource *resource)
{
	CE_ASSERT(hash_map::has(_materials, resource), "Material not found");
	return hash_map::get(_materials, resource, (Material *)NULL);
}

void MaterialManager::reload_textures(const TextureResource *old_resource, const TextureResource *new_resource)
{
#if CROWN_CAN_RELOAD
	auto cur = hash_map::begin(_materials);
	auto end = hash_map::end(_materials);
	for (; cur != end; ++cur) {
		HASH_MAP_SKIP_HOLE(_materials, cur);

		cur->second->reload_textures(old_resource, new_resource);
	}
#else
	CE_UNUSED_2(old_resource, new_resource);
	CE_NOOP();
#endif
}

void MaterialManager::reload_shaders(const ShaderResource *old_resource, const ShaderResource *new_resource)
{
#if CROWN_CAN_RELOAD
	auto cur = hash_map::begin(_materials);
	auto end = hash_map::end(_materials);
	for (; cur != end; ++cur) {
		HASH_MAP_SKIP_HOLE(_materials, cur);

		Material *m = cur->second;
		if (m->_shader.resource == old_resource) {
			m->_shader = _shader_manager->shader(m->_resource->shader);
		}
	}
#else
	CE_UNUSED_2(old_resource, new_resource);
	CE_NOOP();
#endif
}

} // namespace crown
