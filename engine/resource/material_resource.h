/*
Copyright (c) 2013 Daniele Bartolini, Michele Rossi
Copyright (c) 2012 Daniele Bartolini, Simone Boscaratto

Permission is hereby granted, free of charge, to any person
obtaining a copy of this software and associated documentation
files (the "Software"), to deal in the Software without
restriction, including without limitation the rights to use,
copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the
Software is furnished to do so, subject to the following
conditions:

The above copyright notice and this permission notice shall be
included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
OTHER DEALINGS IN THE SOFTWARE.
*/

#pragma once

#include "types.h"
#include "resource.h"
#include "vector3.h"
#include "color4.h"
#include "bundle.h"
#include "allocator.h"
#include "blob.h"
#include "device.h"
#include "file.h"
#include "resource_manager.h"
#include "material_manager.h"

namespace crown
{

struct MaterialResource
{
	uint32_t version;
	uint32_t _pad;
	StringId64 shader;
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
	void* load(Allocator& allocator, Bundle& bundle, ResourceId id);
	void online(StringId64 id, ResourceManager& rm);
	void offline(StringId64 id, ResourceManager& rm);
	void unload(Allocator& a, void* res);

	uint32_t dynamic_data_size(const MaterialResource* mr);
	uint32_t dynamic_data_offset(const MaterialResource* mr);
	StringId64 shader(const MaterialResource* mr);
	uint32_t num_textures(const MaterialResource* mr);
	uint32_t num_uniforms(const MaterialResource* mr);
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
