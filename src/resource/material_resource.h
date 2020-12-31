/*
 * Copyright (c) 2012-2021 Daniele Bartolini et al.
 * License: https://github.com/dbartolini/crown/blob/master/LICENSE
 */

#pragma once

#include "core/filesystem/types.h"
#include "core/math/types.h"
#include "core/memory/types.h"
#include "core/strings/string_id.h"
#include "resource/types.h"

namespace crown
{
struct MaterialResource
{
	u32 version;
	StringId32 shader;
	u32 num_textures;
	u32 texture_data_offset;
	u32 num_uniforms;
	u32 uniform_data_offset;
	u32 dynamic_data_size;
	u32 dynamic_data_offset;
};

struct TextureData
{
	u32 sampler_name_offset; // Sampler uniform name
	StringId32 name;         // Sampler name
	StringId64 id;           // Resource name
	u32 data_offset;         // Offset into dynamic blob
	u32 _pad1;
};

struct TextureHandle
{
	u32 sampler_handle;
	u32 texture_handle;
};

struct UniformType
{
	enum Enum
	{
		FLOAT,
		VECTOR2,
		VECTOR3,
		VECTOR4,
		MATRIX4X4,

		COUNT
	};
};

struct UniformData
{
	u32 type;        // UniformType::Enum
	StringId32 name; // Uniform name
	u32 name_offset; // Uniform name (string)
	u32 data_offset; // Offset into dynamic blob
};

struct UniformHandle
{
	u32 uniform_handle;
	// data
};

namespace material_resource_internal
{
	s32 compile(CompileOptions& opts);
	void* load(File& file, Allocator& a);
	void online(StringId64 id, ResourceManager& rm);
	void offline(StringId64 id, ResourceManager& rm);
	void unload(Allocator& a, void* res);

} // namespace material_resource_internal

namespace material_resource
{
	///
	UniformData* uniform_data(const MaterialResource* mr, u32 i);

	///
	UniformData* uniform_data_by_name(const MaterialResource* mr, StringId32 name);

	///
	const char* uniform_name(const MaterialResource* mr, const UniformData* ud);

	///
	TextureData* texture_data(const MaterialResource* mr, u32 i);

	///
	const char* texture_name(const MaterialResource* mr, const TextureData* td);

	///
	UniformHandle* uniform_handle(const MaterialResource* mr, u32 i, char* dynamic);

	///
	UniformHandle* uniform_handle_by_name(const MaterialResource* mr, StringId32 name, char* dynamic);

	///
	TextureHandle* texture_handle(const MaterialResource* mr, u32 i, char* dynamic);

} // namespace material_resource

} // namespace crown
