/*
 * Copyright (c) 2012-2016 Daniele Bartolini and individual contributors.
 * License: https://github.com/taylor001/crown/blob/master/LICENSE
 */

#include "material.h"
#include "material_resource.h"
#include "memory.h"
#include "device.h"
#include "resource_manager.h"
#include "texture_resource.h"
#include "material_manager.h"
#include "shader.h"
#include <string.h> // memcpy
#include <bgfx/bgfx.h>

namespace crown
{

using namespace material_resource;

void Material::create(const MaterialResource* mr, MaterialManager& mm)
{
	uint32_t size = dynamic_data_size(mr);
	uint32_t offt = dynamic_data_offset(mr);
	char* base = (char*) mr + offt;
	data = (char*) default_allocator().allocate(size);
	memcpy(data, base, size);
	resource = mr;
}

void Material::destroy() const
{
	default_allocator().deallocate(data);
}

void Material::bind() const
{
	// Set samplers
	for (uint32_t i = 0; i < num_textures(resource); i++)
	{
		TextureData* td = get_texture_data(resource, i);
		TextureHandle* th = get_texture_handle(resource, i, data);

		bgfx::UniformHandle sampler;
		bgfx::TextureHandle texture;
		sampler.idx = th->sampler_handle;

		TextureResource* teximg = (TextureResource*) device()->resource_manager()->get(TEXTURE_TYPE, td->id);
		texture.idx = teximg->handle.idx;

		bgfx::setTexture(i, sampler, texture);
	}

	// Set uniforms
	for (uint32_t i = 0; i < num_uniforms(resource); i++)
	{
		UniformHandle* uh = get_uniform_handle(resource, i, data);

		bgfx::UniformHandle buh;
		buh.idx = uh->uniform_handle;
		bgfx::setUniform(buh, (char*) uh + sizeof(uh->uniform_handle));
	}

	Shader* shader = (Shader*) device()->resource_manager()->get(SHADER_TYPE, material_resource::shader(resource));
	bgfx::submit(0, shader->program);
}

void Material::set_float(const char* name, float val)
{
	char* p = (char*) get_uniform_handle_by_string(resource, name, data);
	*((float*)(p + sizeof(uint32_t))) = val;
}

void Material::set_vector2(const char* name, const Vector2& val)
{
	char* p = (char*) get_uniform_handle_by_string(resource, name, data);
	*((Vector2*)(p + sizeof(uint32_t))) = val;
}

void Material::set_vector3(const char* name, const Vector3& val)
{
	char* p = (char*) get_uniform_handle_by_string(resource, name, data);
	*((Vector3*)(p + sizeof(uint32_t))) = val;
}

} // namespace crown
