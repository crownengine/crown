/*
 * Copyright (c) 2012-2024 Daniele Bartolini et al.
 * SPDX-License-Identifier: MIT
 */

#include "core/containers/hash_map.inl"
#include "core/filesystem/file.h"
#include "core/strings/string_id.inl"
#include "resource/material_resource.h"
#include "resource/resource_manager.h"
#include "world/material_manager.h"
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

MaterialManager::MaterialManager(Allocator &a)
	: _allocator(&a)
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

	char *base = (char *)mr + mr->dynamic_data_offset;

	for (u32 i = 0; i < mr->num_textures; ++i) {
		TextureData *td    = texture_data(mr, i);
		TextureHandle *th  = texture_handle(mr, i, base);
		th->sampler_handle = bgfx::createUniform(texture_name(mr, td), bgfx::UniformType::Sampler).idx;
	}

	for (u32 i = 0; i < mr->num_uniforms; ++i) {
		UniformData *ud    = uniform_data(mr, i);
		UniformHandle *uh  = uniform_handle(mr, i, base);
		uh->uniform_handle = bgfx::createUniform(uniform_name(mr, ud), s_bgfx_uniform_type[ud->type]).idx;
	}

	create_material(mr);
}

void MaterialManager::offline(StringId64 id, ResourceManager &rm)
{
	using namespace material_resource;

	MaterialResource *mr = (MaterialResource *)rm.get(RESOURCE_TYPE_MATERIAL, id);

	destroy_material(mr);

	char *base = (char *)mr + mr->dynamic_data_offset;

	for (u32 i = 0; i < mr->num_textures; ++i) {
		TextureHandle *th = texture_handle(mr, i, base);
		bgfx::UniformHandle sh;
		sh.idx = th->sampler_handle;
		bgfx::destroy(sh);
	}

	for (u32 i = 0; i < mr->num_uniforms; ++i) {
		UniformHandle *uh = uniform_handle(mr, i, base);
		bgfx::UniformHandle bgfx_uh;
		bgfx_uh.idx = uh->uniform_handle;
		bgfx::destroy(bgfx_uh);
	}
}

Material *MaterialManager::create_material(const MaterialResource *resource)
{
	Material *mat = hash_map::get(_materials, resource, (Material *)NULL);
	if (mat != NULL)
		return mat;

	const u32 size = sizeof(Material) + resource->dynamic_data_size;
	mat = (Material *)_allocator->allocate(size);
	mat->_resource = resource;
	mat->_data     = (char *)&mat[1];

	const char *data = (char *)resource + resource->dynamic_data_offset;
	memcpy(mat->_data, data, resource->dynamic_data_size);

	hash_map::set(_materials, resource, mat);
	return mat;
}

void MaterialManager::destroy_material(const MaterialResource *resource)
{
	Material *mat = hash_map::get(_materials, resource, (Material *)NULL);
	_allocator->deallocate(mat);

	hash_map::remove(_materials, resource);
}

Material *MaterialManager::get(const MaterialResource *resource)
{
	CE_ASSERT(hash_map::has(_materials, resource), "Material not found");
	return hash_map::get(_materials, resource, (Material *)NULL);
}

} // namespace crown
