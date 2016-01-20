/*
 * Copyright (c) 2012-2016 Daniele Bartolini and individual contributors.
 * License: https://github.com/taylor001/crown/blob/master/LICENSE
 */

#include "material.h"
#include "material_resource.h"
#include "resource_manager.h"
#include "shader_manager.h"
#include "texture_resource.h"
#include <bgfx/bgfx.h>

namespace crown
{

void Material::bind(ResourceManager& rm, ShaderManager& sm) const
{
	using namespace material_resource;

	// Set samplers
	for (uint32_t i = 0; i < _resource->num_textures; ++i)
	{
		const TextureData* td   = get_texture_data(_resource, i);
		const TextureHandle* th = get_texture_handle(_resource, i, _data);

		const TextureResource* teximg = (TextureResource*)rm.get(TEXTURE_TYPE, td->id);

		bgfx::UniformHandle sampler;
		bgfx::TextureHandle texture;
		sampler.idx = th->sampler_handle;
		texture.idx = teximg->handle.idx;

		bgfx::setTexture(i, sampler, texture);
	}

	// Set uniforms
	for (uint32_t i = 0; i < _resource->num_uniforms; ++i)
	{
		const UniformHandle* uh = get_uniform_handle(_resource, i, _data);

		bgfx::UniformHandle buh;
		buh.idx = uh->uniform_handle;
		bgfx::setUniform(buh, (char*)uh + sizeof(uh->uniform_handle));
	}

	const ShaderData& sd = sm.get(_resource->shader);
	bgfx::setState(sd.state);
	bgfx::submit(0, sd.program);
}

void Material::set_float(const char* name, float val)
{
	using namespace material_resource;
	char* p = (char*)get_uniform_handle_by_string(_resource, name, _data);
	*((float*)(p + sizeof(uint32_t))) = val;
}

void Material::set_vector2(const char* name, const Vector2& val)
{
	using namespace material_resource;
	char* p = (char*)get_uniform_handle_by_string(_resource, name, _data);
	*((Vector2*)(p + sizeof(uint32_t))) = val;
}

void Material::set_vector3(const char* name, const Vector3& val)
{
	using namespace material_resource;
	char* p = (char*)get_uniform_handle_by_string(_resource, name, _data);
	*((Vector3*)(p + sizeof(uint32_t))) = val;
}

} // namespace crown
