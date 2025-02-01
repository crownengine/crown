/*
 * Copyright (c) 2012-2025 Daniele Bartolini et al.
 * SPDX-License-Identifier: MIT
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
	UniformData *uniform_data_array(const MaterialResource *mr)
	{
		return (UniformData *)((char *)mr + mr->uniform_data_offset);
	}

	u32 uniform_data_index(const MaterialResource *mr, const UniformData *ud, StringId32 name)
	{
		for (u32 i = 0, n = mr->num_uniforms; i < n; ++i) {
			if (ud[i].name == name)
				return i;
		}

		CE_FATAL("Unknown uniform");
		return UINT32_MAX;
	}

	const char *uniform_name(const MaterialResource *mr, const UniformData *ud, u32 i)
	{
		return (const char *)mr + mr->names_data_offset + ud[i].name_offset;
	}

	TextureData *texture_data_array(const MaterialResource *mr)
	{
		return (TextureData *)((char *)mr + mr->texture_data_offset);
	}

	u32 texture_data_index(const MaterialResource *mr, const TextureData *td, StringId32 name)
	{
		for (u32 i = 0, n = mr->num_textures; i < n; ++i) {
			if (td[i].name == name)
				return i;
		}

		CE_FATAL("Unknown texture");
		return UINT32_MAX;
	}

	const char *texture_name(const MaterialResource *mr, const TextureData *td, u32 i)
	{
		return (const char *)mr + mr->names_data_offset + td[i].sampler_name_offset;
	}

	UniformHandle *uniform_handle(const UniformData *ud, u32 i, char *dynamic)
	{
		return (UniformHandle *)(dynamic + ud[i].data_offset);
	}

	TextureHandle *texture_handle(const TextureData *td, u32 i, char *dynamic)
	{
		return (TextureHandle *)(dynamic + td[i].data_offset);
	}

} // namespace material_resource

namespace material_resource_internal
{
	void online(StringId64 id, ResourceManager &rm)
	{
		device()->_material_manager->online(id, rm);
	}

	void offline(StringId64 id, ResourceManager &rm)
	{
		device()->_material_manager->offline(id, rm);
	}

} // namespace material_resource_internal

#if CROWN_CAN_COMPILE
namespace material_resource_internal
{
	struct UniformTypeInfo
	{
		const char *name;
		UniformType::Enum type;
	};

	static const UniformTypeInfo s_uniform_type_info[] =
	{
		{ "float",     UniformType::FLOAT     },
		{ "vector2",   UniformType::VECTOR2   },
		{ "vector3",   UniformType::VECTOR3   },
		{ "vector4",   UniformType::VECTOR4   },
		{ "matrix4x4", UniformType::MATRIX4X4 }
	};
	CE_STATIC_ASSERT(countof(s_uniform_type_info) == UniformType::COUNT);

	static UniformType::Enum name_to_uniform_type(const char *name)
	{
		for (u32 i = 0; i < countof(s_uniform_type_info); ++i) {
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
	template<typename T>
	static u32 reserve_dynamic_data(Array<char> &dynamic, T data)
	{
		u32 offt = array::size(dynamic);
		array::push(dynamic, (char *)&data, sizeof(data));
		return offt;
	}

	static s32 parse_textures(Array<TextureData> &textures, Array<char> &names, Array<char> &dynamic, const char *json, CompileOptions &opts)
	{
		TempAllocator4096 ta;
		JsonObject obj(ta);
		RETURN_IF_ERROR(sjson::parse(obj, json), opts);

		auto cur = json_object::begin(obj);
		auto end = json_object::end(obj);
		for (; cur != end; ++cur) {
			JSON_OBJECT_SKIP_HOLE(obj, cur);

			const StringView key = cur->first;
			const char *value    = cur->second;

			DynamicString texture(ta);
			RETURN_IF_ERROR(sjson::parse_string(texture, value), opts);
			RETURN_IF_RESOURCE_MISSING("texture", texture.c_str(), opts);
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
			td.id                  = RETURN_IF_ERROR(sjson::parse_resource_name(value), opts);
			td.data_offset         = reserve_dynamic_data(dynamic, th);
			td._pad1               = 0;

			array::push_back(textures, td);
		}

		return 0;
	}

	static s32 parse_uniforms(Array<UniformData> &uniforms, Array<char> &names, Array<char> &dynamic, const char *json, CompileOptions &opts)
	{
		TempAllocator4096 ta;
		JsonObject obj(ta);
		RETURN_IF_ERROR(sjson::parse(obj, json), opts);

		auto cur = json_object::begin(obj);
		auto end = json_object::end(obj);
		for (; cur != end; ++cur) {
			JSON_OBJECT_SKIP_HOLE(obj, cur);

			const StringView key = cur->first;
			const char *value    = cur->second;

			UniformHandle uh;
			uh.uniform_handle = 0;

			JsonObject uniform(ta);
			RETURN_IF_ERROR(sjson::parse_object(uniform, value), opts);

			DynamicString type(ta);
			RETURN_IF_ERROR(sjson::parse_string(type, uniform["type"]), opts);

			const UniformType::Enum ut = name_to_uniform_type(type.c_str());
			RETURN_IF_FALSE(ut != UniformType::COUNT
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

			switch (ud.type) {
			case UniformType::FLOAT: {
				const f32 value = RETURN_IF_ERROR(sjson::parse_float(uniform["value"]), opts);
				Vector4 data;
				data.x = value;
				data.y = 0.0f;
				data.z = 0.0f;
				data.w = 0.0f;
				reserve_dynamic_data(dynamic, data);
				break;
			}

			case UniformType::VECTOR2: {
				const Vector2 value = RETURN_IF_ERROR(sjson::parse_vector2(uniform["value"]), opts);
				Vector4 data;
				data.x = value.x;
				data.y = value.y;
				data.z = 0.0f;
				data.w = 0.0f;
				reserve_dynamic_data(dynamic, data);
				break;
			}

			case UniformType::VECTOR3: {
				const Vector3 value = RETURN_IF_ERROR(sjson::parse_vector3(uniform["value"]), opts);
				Vector4 data;
				data.x = value.x;
				data.y = value.y;
				data.z = value.z;
				data.w = 0.0f;
				reserve_dynamic_data(dynamic, data);
				break;
			}

			case UniformType::VECTOR4: {
				auto data = RETURN_IF_ERROR(sjson::parse_vector4(uniform["value"]), opts);
				reserve_dynamic_data(dynamic, data);
				break;
			}

			case UniformType::MATRIX4X4: {
				auto data = RETURN_IF_ERROR(sjson::parse_matrix4x4(uniform["value"]), opts);
				reserve_dynamic_data(dynamic, data);
				break;
			}

			default:
				CE_FATAL("Unknown uniform type");
				break;
			}

			array::push_back(uniforms, ud);
		}

		return 0;
	}

	s32 compile(CompileOptions &opts)
	{
		Buffer buf = opts.read();
		TempAllocator4096 ta;
		JsonObject obj(ta);
		RETURN_IF_ERROR(sjson::parse(obj, buf), opts);

		Array<TextureData> texdata(default_allocator());
		Array<UniformData> unidata(default_allocator());
		Array<char> names(default_allocator());
		Array<char> dynblob(default_allocator());

		opts.add_requirement_glob("*.shader");

		DynamicString shader(ta);
		RETURN_IF_ERROR(sjson::parse_string(shader, obj["shader"]), opts);

		if (json_object::has(obj, "textures")) {
			s32 err = parse_textures(texdata, names, dynblob, obj["textures"], opts);
			ENSURE_OR_RETURN(err == 0, opts);
		}
		if (json_object::has(obj, "uniforms")) {
			s32 err = parse_uniforms(unidata, names, dynblob, obj["uniforms"], opts);
			ENSURE_OR_RETURN(err == 0, opts);
		}

		MaterialResource mr;
		mr.version             = RESOURCE_HEADER(RESOURCE_VERSION_MATERIAL);
		mr.shader              = shader.to_string_id();
		mr.num_textures        = array::size(texdata);
		mr.texture_data_offset = sizeof(mr);
		mr.num_uniforms        = array::size(unidata);
		mr.uniform_data_offset = mr.texture_data_offset + sizeof(TextureData)*array::size(texdata);
		mr.names_data_size     = array::size(names);
		mr.names_data_offset   = mr.uniform_data_offset + sizeof(UniformData)*array::size(unidata);
		mr.dynamic_data_size   = array::size(dynblob);
		mr.dynamic_data_offset = mr.names_data_offset + mr.names_data_size;

		// Write
		opts.write(mr.version);
		opts.write(mr.shader);
		opts.write(mr.num_textures);
		opts.write(mr.texture_data_offset);
		opts.write(mr.num_uniforms);
		opts.write(mr.uniform_data_offset);
		opts.write(mr.names_data_size);
		opts.write(mr.names_data_offset);
		opts.write(mr.dynamic_data_size);
		opts.write(mr.dynamic_data_offset);

		for (u32 i = 0; i < array::size(texdata); i++) {
			opts.write(texdata[i].sampler_name_offset);
			opts.write(texdata[i].name._id);
			opts.write(texdata[i].id);
			opts.write(texdata[i].data_offset);
			opts.write(texdata[i]._pad1);
		}

		for (u32 i = 0; i < array::size(unidata); i++) {
			opts.write(unidata[i].type);
			opts.write(unidata[i].name);
			opts.write(unidata[i].name_offset);
			opts.write(unidata[i].data_offset);
		}

		opts.write(names);
		opts.write(dynblob);

		return 0;
	}

} // namespace material_resource_internal
#endif // if CROWN_CAN_COMPILE

} // namespace crown
