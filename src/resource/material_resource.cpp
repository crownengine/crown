/*
 * Copyright (c) 2012-2026 Daniele Bartolini et al.
 * SPDX-License-Identifier: MIT
 */

#include "core/containers/array.inl"
#include "core/containers/vector.inl"
#include "core/filesystem/file_buffer.inl"
#include "core/filesystem/filesystem.h"
#include "core/filesystem/reader_writer.h"
#include "core/json/json_object.inl"
#include "core/json/sjson.h"
#include "core/math/constants.h"
#include "core/memory/temp_allocator.inl"
#include "core/strings/dynamic_string.inl"
#include "core/strings/string.inl"
#include "core/strings/string_id.inl"
#include "core/strings/string_view.inl"
#include "device/device.h"
#include "resource/compile_options.inl"
#include "resource/material_resource.h"
#include "resource/resource_manager.h"
#include "resource/shader_resource.h"
#include "world/material_manager.h"

namespace crown
{
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

	union UniformValue
	{
		Vector4 v;
		Matrix4x4 m;
	};

	struct Data
	{
		Vector<UniformMetadata> uniforms_meta;
		Array<TextureData> textures;
		Array<TextureHandle> texture_handles;
		Array<UniformData> uniforms;
		Array<UniformHandle> uniform_handles;
		Array<UniformValue> uniform_values;
		Array<char> names;
		Array<char> dynamic;

		explicit Data(Allocator &a)
			: uniforms_meta(a)
			, textures(a)
			, texture_handles(a)
			, uniforms(a)
			, uniform_handles(a)
			, uniform_values(a)
			, names(a)
			, dynamic(a)
		{
		}

		template<typename T>
		u32 reserve_dynamic_data(T data)
		{
			u32 offt = array::size(dynamic);
			array::push(dynamic, (char *)&data, sizeof(data));
			return offt;
		}

		UniformMetadata *find_meta(const StringView &name)
		{
			for (u32 i = 0; i < vector::size(uniforms_meta); ++i) {
				if (uniforms_meta[i].name == name)
					return &uniforms_meta[i];
			}

			return NULL;
		}

		u32 uniform_index(const StringView &name)
		{
			for (u32 i = 0; i < array::size(uniforms); ++i) {
				if (uniforms[i].name == StringId32(name.data(), name.length()))
					return i;
			}

			return UINT32_MAX;
		}

		void add_shader_uniforms()
		{
			for (u32 i = 0; i < vector::size(uniforms_meta); ++i) {
				UniformMetadata &um = uniforms_meta[i];
				add_uniform(StringView(um.name), um.type, { um.val });
			}
		}

		u32 add_name(const StringView &name)
		{
			const u32 offset = array::size(names);
			array::push(names, name.data(), name.length());
			array::push_back(names, '\0');
			return offset;
		}

		void add_texture(const StringView &sampler, StringId64 texture_name)
		{
			TextureHandle th;
			th.sampler_handle = 0;
			th.texture_handle = 0;

			TextureData td;
			td.sampler_name_offset = add_name(sampler);
			td.name                = StringId32(sampler.data(), sampler.length());
			td.id                  = texture_name;
			td.data_offset         = 0; // Computed later in alloc_blob().
			td._pad1               = 0;

			array::push_back(textures, td);
			array::push_back(texture_handles, th);
		}

		void add_uniform(const StringView &name, UniformType::Enum type, const UniformValue &value)
		{
			UniformHandle uh;
			uh.uniform_handle = 0;

			UniformData ud;
			ud.type        = type;
			ud.name        = StringId32(name.data(), name.length());
			ud.name_offset = add_name(name);
			ud.data_offset = 0; // Computed later in alloc_blob().

			array::push_back(uniforms, ud);
			array::push_back(uniform_handles, uh);
			array::push_back(uniform_values, value);
		}

		s32 check_uniform(const StringView &name, UniformType::Enum type, const UniformValue &value, CompileOptions &opts)
		{
			UniformMetadata *meta = find_meta(name);

			if (meta != NULL) {
				// Uniform has been added already by add_shader_uniforms().
				// Just check for type or range mismatch and overwrite value.
				RETURN_IF_FALSE(type == meta->type, opts, "Uniform %s: type mismatch", meta->name);
				u32 in = uniform_index(name);
				CE_ENSURE(in != UINT32_MAX);
				uniform_values[in] = value;
			} else {
				add_uniform(name, type, value);
			}

			return 0;
		}

		void alloc_blob()
		{
			for (u32 i = 0; i < array::size(textures); ++i)
				textures[i].data_offset = reserve_dynamic_data(texture_handles[i]);

			for (u32 i = 0; i < array::size(uniforms); ++i) {
				uniforms[i].data_offset = reserve_dynamic_data(uniform_handles[i]);
				if (uniforms[i].type == UniformType::MATRIX4X4)
					reserve_dynamic_data(uniform_values[i].m);
				else
					reserve_dynamic_data(uniform_values[i].v);
			}
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

	static s32 parse_textures(Data &data, const char *json, CompileOptions &opts)
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

			data.add_texture(key, StringId64(texture.c_str()));
		}

		return 0;
	}

	static s32 parse_uniforms(Data &data, const char *json, CompileOptions &opts)
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

			UniformValue val;
			memset(&val, 0, sizeof(val));
			s32 err = 0;

			switch (ut) {
			case UniformType::FLOAT:
				val.v.x = RETURN_IF_ERROR(sjson::parse_float(uniform["value"]), opts);
				err = data.check_uniform(key, ut, val, opts);
				break;

			case UniformType::VECTOR2: {
				const Vector2 v = RETURN_IF_ERROR(sjson::parse_vector2(uniform["value"]), opts);
				val.v.x = v.x;
				val.v.y = v.y;
				err = data.check_uniform(key, ut, val, opts);
				break;
			}

			case UniformType::VECTOR3: {
				const Vector3 v = RETURN_IF_ERROR(sjson::parse_vector3(uniform["value"]), opts);
				val.v.x = v.x;
				val.v.y = v.y;
				val.v.z = v.z;
				err = data.check_uniform(key, ut, val, opts);
				break;
			}

			case UniformType::VECTOR4:
				val.v = RETURN_IF_ERROR(sjson::parse_vector4(uniform["value"]), opts);
				err = data.check_uniform(key, ut, val, opts);
				break;

			case UniformType::MATRIX4X4:
				val.m = RETURN_IF_ERROR(sjson::parse_matrix4x4(uniform["value"]), opts);
				err = data.check_uniform(key, ut, val, opts);
				break;

			default:
				CE_FATAL("Unknown uniform type");
				break;
			}

			ENSURE_OR_RETURN(err == 0, opts);
		}

		return 0;
	}

	/// Extracts @a shader_name and a list of @a defines from a @a shader variant string.
	static void shader_name_defines(StringView &shader_name, Vector<StringView> &defines, const char *shader)
	{
		const char *str = shader;

		// Skip shader name.
		const char *sep = strchr(str, '+');
		if (sep == NULL) {
			shader_name = StringView(str, strlen32(str));
			return;
		}

		shader_name = StringView(str, sep - str);
		str = sep + 1;

		// Tokenize defines.
		for (;;) {
			if ((sep = strchr(str, '+')) != NULL) {
				vector::push_back(defines, StringView(str, sep - str));
				str = sep + 1;
				continue;
			}

			vector::push_back(defines, StringView(str, strlen32(str)));
			break;
		}
	}

	s32 compile(CompileOptions &opts)
	{
		Buffer buf = opts.read();
		TempAllocator4096 ta;
		JsonObject obj(ta);
		RETURN_IF_ERROR(sjson::parse(obj, buf), opts);

		Data data(default_allocator());
		Vector<StringView> defines(default_allocator());
		Buffer shader_code(default_allocator());
		FileBuffer shader_fb(shader_code);
		StringView shader_name;
		DynamicString shader_library(ta);
		DynamicString shader(ta);

		RETURN_IF_ERROR(sjson::parse_string(shader, obj["shader"]), opts);
		shader_name_defines(shader_name, defines, shader.c_str());

		shader_compiler::compile_variant(shader_fb, &data.uniforms_meta, shader_library, shader_name, defines, opts, true);
		opts.add_requirement("shader", shader_library.c_str());

		data.add_shader_uniforms();

		// Parse uniforms and textures.
		if (json_object::has(obj, "textures")) {
			s32 err = parse_textures(data, obj["textures"], opts);
			ENSURE_OR_RETURN(err == 0, opts);
		}
		if (json_object::has(obj, "uniforms")) {
			s32 err = parse_uniforms(data, obj["uniforms"], opts);
			ENSURE_OR_RETURN(err == 0, opts);
		}

		data.alloc_blob();

		MaterialResource mr;
		mr.version             = RESOURCE_HEADER(RESOURCE_VERSION_MATERIAL);
		mr.shader              = shader.to_string_id();
		mr.num_textures        = array::size(data.textures);
		mr.texture_data_offset = sizeof(mr);
		mr.num_uniforms        = array::size(data.uniforms);
		mr.uniform_data_offset = mr.texture_data_offset + sizeof(TextureData)*array::size(data.textures);
		mr.names_data_size     = array::size(data.names);
		mr.names_data_offset   = mr.uniform_data_offset + sizeof(UniformData)*array::size(data.uniforms);
		mr.dynamic_data_size   = array::size(data.dynamic);
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
		for (u32 i = 0; i < array::size(data.textures); i++) {
			opts.write(data.textures[i].sampler_name_offset);
			opts.write(data.textures[i].name._id);
			opts.write(data.textures[i].id);
			opts.write(data.textures[i].data_offset);
			opts.write(data.textures[i]._pad1);
		}

		for (u32 i = 0; i < array::size(data.uniforms); i++) {
			opts.write(data.uniforms[i].type);
			opts.write(data.uniforms[i].name);
			opts.write(data.uniforms[i].name_offset);
			opts.write(data.uniforms[i].data_offset);
		}

		opts.write(data.names);
		opts.write(data.dynamic);

		return 0;
	}

} // namespace material_resource_internal
#endif // if CROWN_CAN_COMPILE

} // namespace crown
