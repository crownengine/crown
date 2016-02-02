/*
 * Copyright (c) 2012-2016 Daniele Bartolini and individual contributors.
 * License: https://github.com/taylor001/crown/blob/master/LICENSE
 */

#include "compile_options.h"
#include "device.h"
#include "dynamic_string.h"
#include "filesystem.h"
#include "macros.h"
#include "map.h"
#include "material_manager.h"
#include "material_resource.h"
#include "reader_writer.h"
#include "resource_manager.h"
#include "sjson.h"
#include "string_utils.h"
#include "vector.h"

namespace crown
{
namespace material_resource
{
	struct UniformTypeInfo
	{
		const char* name;
		UniformType::Enum type;
		u8 size;
	};

	static const UniformTypeInfo s_uniform_type_info[] =
	{
		{ "float",   UniformType::FLOAT,    4 },
		{ "vector2", UniformType::VECTOR2,  8 },
		{ "vector3", UniformType::VECTOR3, 12 },
		{ "vector4", UniformType::VECTOR4, 16 }
	};
	CE_STATIC_ASSERT(CE_COUNTOF(s_uniform_type_info) == UniformType::COUNT);

	static UniformType::Enum name_to_uniform_type(const char* name)
	{
		for (u32 i = 0; i < CE_COUNTOF(s_uniform_type_info); ++i)
		{
			if (strcmp(s_uniform_type_info[i].name, name) == 0)
				return s_uniform_type_info[i].type;
		}

		return UniformType::COUNT;
	}

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
	static u32 reserve_dynamic_data(T data, Array<char>& dynamic)
	{
		u32 offt = array::size(dynamic);
		array::push(dynamic, (char*) &data, sizeof(data));
		return offt;
	}

	static void parse_textures(const char* json, Array<TextureData>& textures, Array<char>& names, Array<char>& dynamic, CompileOptions& opts)
	{
		TempAllocator4096 ta;
		JsonObject object(ta);
		sjson::parse(json, object);

		auto begin = map::begin(object);
		auto end = map::end(object);

		for (; begin != end; ++begin)
		{
			const FixedString key = begin->pair.first;
			const char* value     = begin->pair.second;

			DynamicString texture(ta);
			sjson::parse_string(value, texture);
			RESOURCE_COMPILER_ASSERT(opts.resource_exists(TEXTURE_EXTENSION, texture.c_str())
				, opts
				, "Resource does not exist: '%s.%s'"
				, texture.c_str()
				, TEXTURE_EXTENSION
				);

			TextureHandle th;
			th.sampler_handle = 0;
			th.texture_handle = 0;

			const u32 sampler_name_offset = array::size(names);
			array::push(names, key.data(), key.length());
			array::push_back(names, '\0');

			TextureData td;
			td.sampler_name_offset = sampler_name_offset;
			td.id                  = sjson::parse_resource_id(value);
			td.data_offset         = reserve_dynamic_data(th, dynamic);

			array::push_back(textures, td);
		}
	}

	static void parse_uniforms(const char* json, Array<UniformData>& uniforms, Array<char>& names, Array<char>& dynamic, CompileOptions& opts)
	{
		TempAllocator4096 ta;
		JsonObject object(ta);
		sjson::parse(json, object);

		auto begin = map::begin(object);
		auto end = map::end(object);

		for (; begin != end; ++begin)
		{
			const FixedString key = begin->pair.first;
			const char* value     = begin->pair.second;

			UniformHandle uh;
			uh.uniform_handle = 0;

			JsonObject uniform(ta);
			sjson::parse_object(value, uniform);

			DynamicString type(ta);
			sjson::parse_string(uniform["type"], type);

			const UniformType::Enum ut = name_to_uniform_type(type.c_str());
			RESOURCE_COMPILER_ASSERT(ut != UniformType::COUNT
				, opts
				, "Unknown uniform type: '%s'"
				, type.c_str()
				);

			const u32 name_offset = array::size(names);
			array::push(names, key.data(), key.length());
			array::push_back(names, '\0');

			UniformData ud;
			ud.name_offset = name_offset;
			ud.type        = ut;
			ud.data_offset = reserve_dynamic_data(uh, dynamic);

			switch (ud.type)
			{
				case UniformType::FLOAT:
				{
					f32 data = sjson::parse_float(uniform["value"]);
					reserve_dynamic_data(data, dynamic);
					break;
				}
				case UniformType::VECTOR2:
				{
					Vector2 data = sjson::parse_vector2(uniform["value"]);
					reserve_dynamic_data(data, dynamic);
					break;
				}
				case UniformType::VECTOR3:
				{
					Vector3 data = sjson::parse_vector3(uniform["value"]);
					reserve_dynamic_data(data, dynamic);
					break;
				}
				case UniformType::VECTOR4:
				{
					Vector4 data = sjson::parse_vector4(uniform["value"]);
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
		Buffer buf = opts.read(path);
		TempAllocator4096 ta;
		JsonObject object(ta);
		sjson::parse(buf, object);

		Array<TextureData> texdata(default_allocator());
		Array<UniformData> unidata(default_allocator());
		Array<char> names(default_allocator());
		Array<char> dynblob(default_allocator());

		DynamicString shader(ta);
		sjson::parse_string(object["shader"], shader);

		parse_textures(object["textures"], texdata, names, dynblob, opts);
		parse_uniforms(object["uniforms"], unidata, names, dynblob, opts);

		MaterialResource mr;
		mr.version             = MATERIAL_VERSION;
		mr.shader              = shader.to_string_id();
		mr.num_textures        = array::size(texdata);
		mr.texture_data_offset = sizeof(mr);
		mr.num_uniforms        = array::size(unidata);
		mr.uniform_data_offset = mr.texture_data_offset + sizeof(TextureData)*array::size(texdata);
		mr.dynamic_data_size   = array::size(dynblob);
		mr.dynamic_data_offset = mr.uniform_data_offset + sizeof(UniformData)*array::size(unidata);

		// Write
		opts.write(mr.version);
		opts.write(mr.shader);
		opts.write(mr.num_textures);
		opts.write(mr.texture_data_offset);
		opts.write(mr.num_uniforms);
		opts.write(mr.uniform_data_offset);
		opts.write(mr.dynamic_data_size);
		opts.write(mr.dynamic_data_offset);

		for (u32 i = 0; i < array::size(texdata); i++)
		{
			opts.write(texdata[i].sampler_name_offset);
			opts.write(texdata[i]._pad);
			opts.write(texdata[i].id);
			opts.write(texdata[i].data_offset);
			opts.write(texdata[i]._pad1);
		}

		for (u32 i = 0; i < array::size(unidata); i++)
		{
			opts.write(unidata[i].type);
			opts.write(unidata[i].name_offset);
			opts.write(unidata[i].data_offset);
		}

		opts.write(dynblob);
		opts.write(names);
	}

	void* load(File& file, Allocator& a)
	{
		return device()->material_manager()->load(file, a);
	}

	void online(StringId64 id, ResourceManager& rm)
	{
		device()->material_manager()->online(id, rm);
	}

	void offline(StringId64 id, ResourceManager& rm)
	{
		device()->material_manager()->offline(id, rm);
	}

	void unload(Allocator& a, void* res)
	{
		device()->material_manager()->unload(a, res);
	}

	UniformData* get_uniform_data(const MaterialResource* mr, u32 i)
	{
		UniformData* base = (UniformData*) ((char*)mr + mr->uniform_data_offset);
		return &base[i];
	}

	UniformData* get_uniform_data_by_string(const MaterialResource* mr, const char* str)
	{
		for (u32 i = 0, n = mr->num_uniforms; i < n; ++i)
		{
			UniformData* base = get_uniform_data(mr, i);
			const char* name = get_uniform_name(mr, base);

			if (strcmp(str, name) == 0)
				return base;
		}

		CE_ASSERT(false, "Bad uniform name: '%s'", str);
		return NULL;
	}

	const char* get_uniform_name(const MaterialResource* mr, const UniformData* ud)
	{
		return (const char*)mr + mr->dynamic_data_offset + mr->dynamic_data_size + ud->name_offset;
	}

	TextureData* get_texture_data(const MaterialResource* mr, u32 i)
	{
		TextureData* base = (TextureData*) ((char*)mr + mr->texture_data_offset);
		return &base[i];
	}

	const char* get_texture_name(const MaterialResource* mr, const TextureData* td)
	{
		return (const char*)mr + mr->dynamic_data_offset + mr->dynamic_data_size + td->sampler_name_offset;
	}

	UniformHandle* get_uniform_handle(const MaterialResource* mr, u32 i, char* dynamic)
	{
		UniformData* ud = get_uniform_data(mr, i);
		return (UniformHandle*) (dynamic + ud->data_offset);
	}

	UniformHandle* get_uniform_handle_by_string(const MaterialResource* mr, const char* str, char* dynamic)
	{
		UniformData* ud = get_uniform_data_by_string(mr, str);
		return (UniformHandle*) (dynamic + ud->data_offset);
	}

	TextureHandle* get_texture_handle(const MaterialResource* mr, u32 i, char* dynamic)
	{
		TextureData* td = get_texture_data(mr, i);
		return (TextureHandle*) (dynamic + td->data_offset);
	}
} // namespace material_resource
} // namespace crown
