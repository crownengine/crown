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

#include "material_resource.h"
#include "dynamic_string.h"
#include "string_utils.h"
#include "string_utils.h"
#include "json_parser.h"
#include "filesystem.h"
#include "reader_writer.h"
#include "os.h"

namespace crown
{
namespace material_resource
{
	struct Data
	{
		Array<TextureData> textures;
		Array<UniformData> uniforms;
		Array<char> dynamic;

		Data()
			: textures(default_allocator())
			, uniforms(default_allocator())
			, dynamic(default_allocator())
		{}
	};

	// Returns offset to start of data
	template <typename T>
	static uint32_t reserve_dynamic_data(T data, Array<char>& dynamic)
	{
		uint32_t offt = array::size(dynamic);
		array::push(dynamic, (char*) &data, sizeof(data));
		return offt;
	}

	static void parse_textures(JSONElement root, Array<TextureData>& textures, Array<char>& names, Array<char>& dynamic)
	{
		using namespace vector;

		Vector<DynamicString> keys(default_allocator());
		root.key("textures").to_keys(keys);

		for (uint32_t i = 0; i < size(keys); i++)
		{
			TextureHandle th;
			th.sampler_handle = 0;
			th.texture_handle = 0;

			ResourceId texid = root.key("textures").key(keys[i].c_str()).to_resource_id("texture");

			TextureData td;
			td.sampler_name_offset = array::size(names); array::push(names, keys[i].c_str(), keys[i].length()); array::push_back(names, '\0');
			td.id = texid.name;
			td.data_offset = reserve_dynamic_data(th, dynamic);

			array::push_back(textures, td);
		}
	}

	struct UniformTypeInfo
	{
		const char* name;
		UniformType::Enum type;
		uint8_t size;
	};

	static const UniformTypeInfo s_uniform_type_info[UniformType::COUNT] =
	{
		{ "float",   UniformType::FLOAT,    4 },
		{ "vector2", UniformType::VECTOR2,  8 },
		{ "vector3", UniformType::VECTOR3, 12 },
		{ "vector4", UniformType::VECTOR4, 16 }
	};

	static UniformType::Enum string_to_uniform_type(const char* str)
	{
		for (uint32_t i = 0; i < UniformType::COUNT; i++)
		{
			if (string::strcmp(str, s_uniform_type_info[i].name) == 0)
				return s_uniform_type_info[i].type;
		}

		CE_FATAL("Unknown uniform type");
		return UniformType::COUNT;
	}

	static void parse_uniforms(JSONElement root, Array<UniformData>& uniforms, Array<char>& names, Array<char>& dynamic)
	{
		using namespace vector;

		Vector<DynamicString> keys(default_allocator());
		root.key("uniforms").to_keys(keys);

		for (uint32_t i = 0; i < size(keys); i++)
		{
			UniformHandle uh;
			uh.uniform_handle = 0;

			DynamicString type;
			root.key("uniforms").key(keys[i].c_str()).key("type").to_string(type);

			UniformData ud;
			ud.name_offset = array::size(names); array::push(names, keys[i].c_str(), keys[i].length()); array::push_back(names, '\0');
			ud.type = string_to_uniform_type(type.c_str());
			ud.data_offset = reserve_dynamic_data(uh, dynamic);

			switch (ud.type)
			{
				case UniformType::FLOAT:
				{
					float data = root.key("uniforms").key(keys[i].c_str()).key("value").to_float();
					reserve_dynamic_data(data, dynamic);
					break;
				}
				case UniformType::VECTOR2:
				{
					Vector2 data = root.key("uniforms").key(keys[i].c_str()).key("value").to_vector2();
					reserve_dynamic_data(data, dynamic);
					break;
				}
				case UniformType::VECTOR3:
				{
					Vector3 data = root.key("uniforms").key(keys[i].c_str()).key("value").to_vector3();
					reserve_dynamic_data(data, dynamic);
					break;
				}
				case UniformType::VECTOR4:
				{
					Vector4 data = root.key("uniforms").key(keys[i].c_str()).key("value").to_vector4();
					reserve_dynamic_data(data, dynamic);
					break;
				}
				default: CE_FATAL("Oops"); break;
			}

			array::push_back(uniforms, ud);
		}
	}

	void compile(const char* path, CompileOptions& opts)
	{
		static const uint32_t VERSION = 1;

		Buffer buf = opts.read(path);
		JSONParser json(array::begin(buf));
		JSONElement root = json.root();

		Array<TextureData> texdata(default_allocator());
		Array<UniformData> unidata(default_allocator());
		Array<char> names(default_allocator());
		Array<char> dynblob(default_allocator());

		ResourceId shader = root.key("shader").to_resource_id("shader");
		parse_textures(root, texdata, names, dynblob);
		parse_uniforms(root, unidata, names, dynblob);

		MaterialResource mr;
		mr.version = VERSION;
		mr.shader = shader.name;
		mr.num_textures = array::size(texdata);
		mr.texture_data_offset = sizeof(mr);
		mr.num_uniforms = array::size(unidata);
		mr.uniform_data_offset = sizeof(mr) + sizeof(TextureData) * array::size(texdata);
		mr.dynamic_data_size = array::size(dynblob);
		mr.dynamic_data_offset = sizeof(mr) + sizeof(TextureData) * array::size(texdata) + sizeof(UniformData) * array::size(unidata);

		// Write
		opts.write(mr.version);
		opts.write(mr._pad);
		opts.write(mr.shader);
		opts.write(mr.num_textures);
		opts.write(mr.texture_data_offset);
		opts.write(mr.num_uniforms);
		opts.write(mr.uniform_data_offset);
		opts.write(mr.dynamic_data_size);
		opts.write(mr.dynamic_data_offset);

		for (uint32_t i = 0; i < array::size(texdata); i++)
		{
			opts.write(texdata[i].sampler_name_offset);
			opts.write(texdata[i]._pad);
			opts.write(texdata[i].id);
			opts.write(texdata[i].data_offset);
			opts.write(texdata[i]._pad1);
		}

		for (uint32_t i = 0; i < array::size(unidata); i++)
		{
			opts.write(unidata[i].name_offset);
			opts.write(unidata[i].type);
			opts.write(unidata[i].data_offset);
		}

		opts.write(dynblob);
		opts.write(names);
	}

	void* load(File& file, Allocator& a)
	{
		const size_t file_size = file.size();
		void* res = a.allocate(file_size);
		file.read(res, file_size);
		return res;
	}

	void online(StringId64 id, ResourceManager& rm)
	{
		MaterialResource* mr = (MaterialResource*) rm.get(MATERIAL_TYPE, id);

		char* base = (char*)mr + dynamic_data_offset(mr);

		for (uint32_t i = 0; i < num_textures(mr); i++)
		{
			TextureData* td = get_texture_data(mr, i);
			TextureHandle* th = get_texture_handle(mr, i, base);
			th->sampler_handle = bgfx::createUniform(get_texture_name(mr, td), bgfx::UniformType::Uniform1iv).idx;
		}

		for (uint32_t i = 0; i < num_uniforms(mr); i++)
		{
			UniformData* ud = get_uniform_data(mr, i);
			UniformHandle* uh = get_uniform_handle(mr, i, base);
			uh->uniform_handle = bgfx::createUniform(get_uniform_name(mr, ud), bgfx::UniformType::Uniform4fv).idx;
		}
	}

	void offline(StringId64 id, ResourceManager& rm)
	{
		MaterialResource* mr = (MaterialResource*) rm.get(MATERIAL_TYPE, id);

		char* base = (char*) mr + dynamic_data_offset(mr);

		for (uint32_t i = 0; i < num_textures(mr); i++)
		{
			TextureHandle* th = get_texture_handle(mr, i, base);
			bgfx::UniformHandle sh;
			sh.idx = th->sampler_handle;
			bgfx::destroyUniform(sh);
		}

		for (uint32_t i = 0; i < num_uniforms(mr); i++)
		{
			UniformHandle* uh = get_uniform_handle(mr, i, base);
			bgfx::UniformHandle bgfx_uh;
			bgfx_uh.idx = uh->uniform_handle;
			bgfx::destroyUniform(bgfx_uh);
		}
	}

	void unload(Allocator& a, void* res)
	{
		a.deallocate(res);
	}

	uint32_t dynamic_data_size(const MaterialResource* mr)
	{
		return mr->dynamic_data_size;
	}

	uint32_t dynamic_data_offset(const MaterialResource* mr)
	{
		return mr->dynamic_data_offset;
	}

	StringId64 shader(const MaterialResource* mr)
	{
		return mr->shader;		
	}

	uint32_t num_textures(const MaterialResource* mr)
	{
		return mr->num_textures;
	}

	uint32_t num_uniforms(const MaterialResource* mr)
	{
		return mr->num_uniforms;		
	}

	UniformData* get_uniform_data(const MaterialResource* mr, uint32_t i)
	{
		UniformData* base = (UniformData*) ((char*)mr + mr->uniform_data_offset);
		return &base[i];
	}

	UniformData* get_uniform_data_by_string(const MaterialResource* mr, const char* str)
	{
		UniformData* base = (UniformData*) ((char*)mr + mr->uniform_data_offset);

		uint32_t num = num_uniforms(mr);
		for (uint32_t i = 0; i < num; i++)
		{
			const char* name = get_uniform_name(mr, (const UniformData*)base->name_offset);
			if (string::strcmp(name, str) == 0)
				return base;

			base++;
		}

		CE_FATAL("Oops, bad uniform name");
		return NULL;
	}

	const char* get_uniform_name(const MaterialResource* mr, const UniformData* ud)
	{
		return (const char*)mr + mr->dynamic_data_offset + mr->dynamic_data_size + ud->name_offset;
	}

	TextureData* get_texture_data(const MaterialResource* mr, uint32_t i)
	{
		TextureData* base = (TextureData*) ((char*)mr + mr->texture_data_offset);
		return &base[i];
	}

	const char* get_texture_name(const MaterialResource* mr, const TextureData* td)
	{
		return (const char*)mr + mr->dynamic_data_offset + mr->dynamic_data_size + td->sampler_name_offset;
	}

	UniformHandle* get_uniform_handle(const MaterialResource* mr, uint32_t i, char* dynamic)
	{
		UniformData* ud = get_uniform_data(mr, i);
		return (UniformHandle*) (dynamic + ud->data_offset);
	}

	UniformHandle* get_uniform_handle_by_string(const MaterialResource* mr, const char* str, char* dynamic)
	{
		UniformData* ud = get_uniform_data_by_string(mr, str);
		return (UniformHandle*) (dynamic + ud->data_offset);
	}	

	TextureHandle* get_texture_handle(const MaterialResource* mr, uint32_t i, char* dynamic)
	{
		TextureData* td = get_texture_data(mr, i);
		return (TextureHandle*) (dynamic + td->data_offset);
	}
} // namespace material_resource
} // namespace crown
