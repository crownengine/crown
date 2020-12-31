/*
 * Copyright (c) 2012-2021 Daniele Bartolini et al.
 * License: https://github.com/dbartolini/crown/blob/master/LICENSE
 */

#include "core/containers/array.inl"
#include "core/containers/vector.inl"
#include "core/filesystem/filesystem.h"
#include "core/filesystem/reader_writer.h"
#include "core/json/json_object.inl"
#include "core/json/sjson.h"
#include "core/memory/temp_allocator.inl"
#include "core/strings/dynamic_string.inl"
#include "core/strings/string.inl"
#include "core/strings/string_id.inl"
#include "core/strings/string_view.inl"
#include "device/device.h"
#include "resource/compile_options.inl"
#include "resource/material_resource.h"
#include "resource/resource_manager.h"
#include "world/material_manager.h"

namespace crown
{
namespace material_resource
{
	UniformData* uniform_data(const MaterialResource* mr, u32 i)
	{
		UniformData* base = (UniformData*)((char*)mr + mr->uniform_data_offset);
		return &base[i];
	}

	UniformData* uniform_data_by_name(const MaterialResource* mr, StringId32 name)
	{
		for (u32 i = 0, n = mr->num_uniforms; i < n; ++i)
		{
			UniformData* data = uniform_data(mr, i);
			if (data->name == name)
				return data;
		}

		CE_FATAL("Unknown uniform");
		return NULL;
	}

	const char* uniform_name(const MaterialResource* mr, const UniformData* ud)
	{
		return (const char*)mr + mr->dynamic_data_offset + mr->dynamic_data_size + ud->name_offset;
	}

	TextureData* texture_data(const MaterialResource* mr, u32 i)
	{
		TextureData* base = (TextureData*)((char*)mr + mr->texture_data_offset);
		return &base[i];
	}

	const char* texture_name(const MaterialResource* mr, const TextureData* td)
	{
		return (const char*)mr + mr->dynamic_data_offset + mr->dynamic_data_size + td->sampler_name_offset;
	}

	UniformHandle* uniform_handle(const MaterialResource* mr, u32 i, char* dynamic)
	{
		UniformData* ud = uniform_data(mr, i);
		return (UniformHandle*)(dynamic + ud->data_offset);
	}

	UniformHandle* uniform_handle_by_name(const MaterialResource* mr, StringId32 name, char* dynamic)
	{
		UniformData* ud = uniform_data_by_name(mr, name);
		return (UniformHandle*)(dynamic + ud->data_offset);
	}

	TextureHandle* texture_handle(const MaterialResource* mr, u32 i, char* dynamic)
	{
		TextureData* td = texture_data(mr, i);
		return (TextureHandle*)(dynamic + td->data_offset);
	}

} // namespace material_resource

namespace material_resource_internal
{
	void* load(File& file, Allocator& a)
	{
		return device()->_material_manager->load(file, a);
	}

	void online(StringId64 id, ResourceManager& rm)
	{
		device()->_material_manager->online(id, rm);
	}

	void offline(StringId64 id, ResourceManager& rm)
	{
		device()->_material_manager->offline(id, rm);
	}

	void unload(Allocator& a, void* res)
	{
		device()->_material_manager->unload(a, res);
	}

} // namespace material_resource_internal

#if CROWN_CAN_COMPILE
namespace material_resource_internal
{
	struct UniformTypeInfo
	{
		const char* name;
		UniformType::Enum type;
		u8 size;
	};

	static const UniformTypeInfo s_uniform_type_info[] =
	{
		{ "float",     UniformType::FLOAT,      4 },
		{ "vector2",   UniformType::VECTOR2,    8 },
		{ "vector3",   UniformType::VECTOR3,   12 },
		{ "vector4",   UniformType::VECTOR4,   16 },
		{ "matrix4x4", UniformType::MATRIX4X4, 64 }
	};
	CE_STATIC_ASSERT(countof(s_uniform_type_info) == UniformType::COUNT);

	static UniformType::Enum name_to_uniform_type(const char* name)
	{
		for (u32 i = 0; i < countof(s_uniform_type_info); ++i)
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
		{
		}
	};

	// Returns offset to start of data
	template <typename T>
	static u32 reserve_dynamic_data(Array<char>& dynamic, T data)
	{
		u32 offt = array::size(dynamic);
		array::push(dynamic, (char*) &data, sizeof(data));
		return offt;
	}

	static s32 parse_textures(const char* json, Array<TextureData>& textures, Array<char>& names, Array<char>& dynamic, CompileOptions& opts)
	{
		TempAllocator4096 ta;
		JsonObject obj(ta);
		sjson::parse(obj, json);

		auto cur = json_object::begin(obj);
		auto end = json_object::end(obj);
		for (; cur != end; ++cur)
		{
			JSON_OBJECT_SKIP_HOLE(obj, cur);

			const StringView key = cur->first;
			const char* value    = cur->second;

			DynamicString texture(ta);
			sjson::parse_string(texture, value);
			DATA_COMPILER_ASSERT_RESOURCE_EXISTS("texture", texture.c_str(), opts);
			opts.add_requirement("texture", texture.c_str());

			TextureHandle th;
			th.sampler_handle = 0;
			th.texture_handle = 0;

			const u32 sampler_name_offset = array::size(names);
			array::push(names, key.data(), key.length());
			array::push_back(names, '\0');

			TextureData td;
			td.sampler_name_offset = sampler_name_offset;
			td.name                = StringId32(key.data(), key.length());
			td.id                  = sjson::parse_resource_name(value);
			td.data_offset         = reserve_dynamic_data(dynamic, th);
			td._pad1               = 0;

			array::push_back(textures, td);
		}

		return 0;
	}

	static s32 parse_uniforms(const char* json, Array<UniformData>& uniforms, Array<char>& names, Array<char>& dynamic, CompileOptions& opts)
	{
		TempAllocator4096 ta;
		JsonObject obj(ta);
		sjson::parse(obj, json);

		auto cur = json_object::begin(obj);
		auto end = json_object::end(obj);
		for (; cur != end; ++cur)
		{
			JSON_OBJECT_SKIP_HOLE(obj, cur);

			const StringView key = cur->first;
			const char* value    = cur->second;

			UniformHandle uh;
			uh.uniform_handle = 0;

			JsonObject uniform(ta);
			sjson::parse_object(uniform, value);

			DynamicString type(ta);
			sjson::parse_string(type, uniform["type"]);

			const UniformType::Enum ut = name_to_uniform_type(type.c_str());
			DATA_COMPILER_ASSERT(ut != UniformType::COUNT
				, opts
				, "Unknown uniform type: '%s'"
				, type.c_str()
				);

			const u32 name_offset = array::size(names);
			array::push(names, key.data(), key.length());
			array::push_back(names, '\0');

			UniformData ud;
			ud.type        = ut;
			ud.name        = StringId32(key.data(), key.length());
			ud.name_offset = name_offset;
			ud.data_offset = reserve_dynamic_data(dynamic, uh);

			switch (ud.type)
			{
			case UniformType::FLOAT:
				reserve_dynamic_data(dynamic, sjson::parse_float(uniform["value"]));
				break;

			case UniformType::VECTOR2:
				reserve_dynamic_data(dynamic, sjson::parse_vector2(uniform["value"]));
				break;

			case UniformType::VECTOR3:
				reserve_dynamic_data(dynamic, sjson::parse_vector3(uniform["value"]));
				break;

			case UniformType::VECTOR4:
				reserve_dynamic_data(dynamic, sjson::parse_vector4(uniform["value"]));
				break;

			case UniformType::MATRIX4X4:
				reserve_dynamic_data(dynamic, sjson::parse_matrix4x4(uniform["value"]));
				break;

			default:
				CE_FATAL("Unknown uniform type");
				break;
			}

			array::push_back(uniforms, ud);
		}

		return 0;
	}

	s32 compile(CompileOptions& opts)
	{
		Buffer buf = opts.read();
		TempAllocator4096 ta;
		JsonObject obj(ta);
		sjson::parse(obj, buf);

		Array<TextureData> texdata(default_allocator());
		Array<UniformData> unidata(default_allocator());
		Array<char> names(default_allocator());
		Array<char> dynblob(default_allocator());

		DynamicString shader(ta);
		sjson::parse_string(shader, obj["shader"]);

		if (json_object::has(obj, "textures"))
		{
			s32 err = parse_textures(obj["textures"], texdata, names, dynblob, opts);
			DATA_COMPILER_ENSURE(err == 0, opts);
		}
		if (json_object::has(obj, "uniforms"))
		{
			s32 err = parse_uniforms(obj["uniforms"], unidata, names, dynblob, opts);
			DATA_COMPILER_ENSURE(err == 0, opts);
		}

		MaterialResource mr;
		mr.version             = RESOURCE_HEADER(RESOURCE_VERSION_MATERIAL);
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
			opts.write(texdata[i].name._id);
			opts.write(texdata[i].id);
			opts.write(texdata[i].data_offset);
			opts.write(texdata[i]._pad1);
		}

		for (u32 i = 0; i < array::size(unidata); i++)
		{
			opts.write(unidata[i].type);
			opts.write(unidata[i].name);
			opts.write(unidata[i].name_offset);
			opts.write(unidata[i].data_offset);
		}

		opts.write(dynblob);
		opts.write(names);

		return 0;
	}

} // namespace material_resource_internal
#endif // CROWN_CAN_COMPILE

} // namespace crown
