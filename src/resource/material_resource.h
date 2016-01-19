/*
 * Copyright (c) 2012-2016 Daniele Bartolini and individual contributors.
 * License: https://github.com/taylor001/crown/blob/master/LICENSE
 */

#pragma once

#include "memory_types.h"
#include "resource_types.h"
#include "filesystem_types.h"
#include "math_types.h"
#include "compiler_types.h"
#include "string_id.h"

namespace crown
{

struct MaterialResource
{
	uint32_t version;
	uint32_t _pad;
	StringId32 shader;
	uint32_t num_textures;
	uint32_t texture_data_offset;
	uint32_t num_uniforms;
	uint32_t uniform_data_offset;
	uint32_t dynamic_data_size;
	uint32_t dynamic_data_offset;
};

struct TextureData
{
	uint32_t sampler_name_offset;	// Sampler uniform name
	uint32_t _pad;
	StringId64 id;					// Resource name
	uint32_t data_offset;			// Offset into dynamic blob
	uint32_t _pad1;
};

struct TextureHandle
{
	uint32_t sampler_handle;
	uint32_t texture_handle;
};

struct UniformType
{
	enum Enum
	{
		FLOAT,
		VECTOR2,
		VECTOR3,
		VECTOR4,
		COUNT
	};
};

struct UniformData
{
	uint32_t name_offset;	// Uniform name
	uint32_t type;			// UniformType::Enum
	uint32_t data_offset;	// Offset into dynamic blob
};

struct UniformHandle
{
	uint32_t uniform_handle;
	// data
};

namespace material_resource
{
	void compile(const char* path, CompileOptions& opts);
	void* load(File& file, Allocator& a);
	void online(StringId64 id, ResourceManager& rm);
	void offline(StringId64 id, ResourceManager& rm);
	void unload(Allocator& a, void* res);

	UniformData* get_uniform_data(const MaterialResource* mr, uint32_t i);
	UniformData* get_uniform_data_by_string(const MaterialResource* mr, const char* str);
	const char* get_uniform_name(const MaterialResource* mr, const UniformData* ud);
	TextureData* get_texture_data(const MaterialResource* mr, uint32_t i);
	const char* get_texture_name(const MaterialResource* mr, const TextureData* td);
	UniformHandle* get_uniform_handle(const MaterialResource* mr, uint32_t i, char* dynamic);
	UniformHandle* get_uniform_handle_by_string(const MaterialResource* mr, const char* str, char* dynamic);
	TextureHandle* get_texture_handle(const MaterialResource* mr, uint32_t i, char* dynamic);
} // namespace material_resource
} // namespace crown
