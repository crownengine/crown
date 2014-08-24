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

const uint32_t MATERIAL_VERSION = 1;

struct MaterialHeader
{
	uint32_t version;
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
	char sampler_name[256];	// Sampler uniform name
	StringId64 id;			// Resource name
	uint32_t data_offset;	// Offset into dynamic blob
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
	char name[256];			// Uniform name
	uint32_t type;			// UniformType::Enum
	uint32_t data_offset;	// Offset into dynamic blob
};

struct UniformHandle
{
	uint32_t uniform_handle;
	// data
};

struct MaterialResource
{
public:

	//-----------------------------------------------------------------------------
	static void* load(Allocator& allocator, Bundle& bundle, ResourceId id)
	{
		File* file = bundle.open(id);
		const size_t file_size = file->size();

		void* res = allocator.allocate(file_size);
		file->read(res, file_size);

		bundle.close(file);

		return res;
	}

	//-----------------------------------------------------------------------------
	static void online(StringId64 id, ResourceManager& rm)
	{
		ResourceId res_id;
		res_id.type = MATERIAL_TYPE;
		res_id.name = id;
		MaterialResource* mr = (MaterialResource*) rm.get(res_id);

		char* base = (char*) mr + mr->dynamic_data_offset();

		for (uint32_t i = 0; i < mr->num_textures(); i++)
		{
			TextureData* ud = mr->get_texture_data(i);
			TextureHandle* th = mr->get_texture_handle(i, base);
			th->sampler_handle = bgfx::createUniform(ud->sampler_name, bgfx::UniformType::Uniform1iv).idx;
		}

		for (uint32_t i = 0; i < mr->num_uniforms(); i++)
		{
			UniformData* ud = mr->get_uniform_data(i);
			UniformHandle* uh = mr->get_uniform_handle(i, base);
			uh->uniform_handle = bgfx::createUniform(ud->name, bgfx::UniformType::Uniform4fv).idx;
		}
	}

	//-----------------------------------------------------------------------------
	static void offline(StringId64 id, ResourceManager& rm)
	{
		ResourceId res_id;
		res_id.type = MATERIAL_TYPE;
		res_id.name = id;
		MaterialResource* mr = (MaterialResource*) rm.get(res_id);

		char* base = (char*) mr + mr->dynamic_data_offset();

		for (uint32_t i = 0; i < mr->num_textures(); i++)
		{
			TextureHandle* th = mr->get_texture_handle(i, base);
			bgfx::UniformHandle sh;
			sh.idx = th->sampler_handle;
			bgfx::destroyUniform(sh);
		}

		for (uint32_t i = 0; i < mr->num_uniforms(); i++)
		{
			UniformHandle* uh = mr->get_uniform_handle(i, base);
			bgfx::UniformHandle bgfx_uh;
			bgfx_uh.idx = uh->uniform_handle;
			bgfx::destroyUniform(bgfx_uh);
		}
	}

	//-----------------------------------------------------------------------------
	static void unload(Allocator& a, void* res)
	{
		a.deallocate(res);
	}

	uint32_t dynamic_data_size() const
	{
		MaterialHeader* mh = (MaterialHeader*) this;
		return mh->dynamic_data_size;		
	}

	uint32_t dynamic_data_offset() const
	{
		MaterialHeader* mh = (MaterialHeader*) this;
		return mh->dynamic_data_offset;
	}

	StringId64 shader() const
	{
		MaterialHeader* mh = (MaterialHeader*) this;
		return mh->shader;		
	}

	uint32_t num_textures() const
	{
		MaterialHeader* mh = (MaterialHeader*) this;
		return mh->num_textures;
	}

	uint32_t num_uniforms() const
	{
		MaterialHeader* mh = (MaterialHeader*) this;
		return mh->num_uniforms;		
	}

	UniformData* get_uniform_data(uint32_t i) const
	{
		MaterialHeader* mh = (MaterialHeader*) this;
		UniformData* base = (UniformData*) ((char*) mh + mh->uniform_data_offset);
		return &base[i];
	}

	UniformData* get_uniform_data_by_string(const char* str) const
	{
		MaterialHeader* mh = (MaterialHeader*) this;
		UniformData* base = (UniformData*) ((char*) mh + mh->uniform_data_offset);

		uint32_t num = num_uniforms();
		for (uint32_t i = 0; i < num; i++)
		{
			if (string::strncmp(base->name, str, 256) == 0)
				return base;

			base++;
		}

		CE_FATAL("Oops, bad uniform name");
		return NULL;
	}

	TextureData* get_texture_data(uint32_t i) const
	{
		MaterialHeader* mh = (MaterialHeader*) this;
		TextureData* base = (TextureData*) ((char*) mh + mh->texture_data_offset);
		return &base[i];
	}

	UniformHandle* get_uniform_handle(uint32_t i, char* dynamic) const
	{
		UniformData* ud = get_uniform_data(i);
		return (UniformHandle*) (dynamic + ud->data_offset);
	}

	UniformHandle* get_uniform_handle_by_string(const char* str, char* dynamic) const
	{
		UniformData* ud = get_uniform_data_by_string(str);
		return (UniformHandle*) (dynamic + ud->data_offset);
	}	

	TextureHandle* get_texture_handle(uint32_t i, char* dynamic) const
	{
		TextureData* td = get_texture_data(i);
		return (TextureHandle*) (dynamic + td->data_offset);
	}

private:

	// Disable construction
	MaterialResource();
};

} // namespace crown
