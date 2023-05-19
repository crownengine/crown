/*
 * Copyright (c) 2012-2023 Daniele Bartolini et al.
 * SPDX-License-Identifier: MIT
 */

#include "config.h"
#include "core/containers/array.inl"
#include "core/containers/hash_map.inl"
#include "core/containers/hash_set.inl"
#include "core/filesystem/file.h"
#include "core/filesystem/file_buffer.inl"
#include "core/filesystem/filesystem.h"
#include "core/filesystem/reader_writer.inl"
#include "core/json/json_object.inl"
#include "core/json/sjson.h"
#include "core/memory/temp_allocator.inl"
#include "core/strings/dynamic_string.inl"
#include "core/strings/string_id.inl"
#include "resource/compile_options.inl"
#include "resource/data_compiler.h"
#include "resource/package_resource.h"
#include "resource/resource_id.inl"

namespace crown
{
template<>
struct hash<ResourceOffset>
{
	u32 operator()(const ResourceOffset &val) const
	{
		return (u32)resource_id(val.type, val.name)._id;
	}
};

bool operator<(const ResourceOffset &a, const ResourceOffset &b)
{
	return a.type < b.type;
}

bool operator==(const ResourceOffset &a, const ResourceOffset &b)
{
	return a.type == b.type
		&& a.name == b.name
		;
}

#if CROWN_CAN_COMPILE
namespace package_resource_internal
{
	s32 bring_in_requirements(HashSet<ResourceOffset> &output
		, CompileOptions &opts
		, ResourceId res_id
		)
	{
		const HashMap<DynamicString, u32> reqs_deffault(default_allocator());
		const HashMap<DynamicString, u32> &reqs = hash_map::get(opts._data_compiler._data_requirements, res_id, reqs_deffault);

		auto cur = hash_map::begin(reqs);
		auto end = hash_map::end(reqs);
		for (; cur != end; ++cur) {
			HASH_MAP_SKIP_HOLE(reqs, cur);

			const char *req_filename = cur->first.c_str();
			const char *req_type = resource_type(req_filename);
			const u32 req_name_len = resource_name_length(req_type, req_filename);

			const StringId64 req_type_hash(req_type);
			const StringId64 req_name_hash(req_filename, req_name_len);

			ResourceOffset ro;
			ro.type = req_type_hash;
			ro.name = req_name_hash;
			hash_set::insert(output, ro);

			bring_in_requirements(output, opts, resource_id(req_type_hash, req_name_hash));
		}

		return 0;
	}

	s32 compile_resources(HashSet<ResourceOffset> &output
		, CompileOptions &opts
		, const char *type
		, const JsonArray &names
		)
	{
		const StringId64 type_hash = StringId64(type);

		for (u32 i = 0; i < array::size(names); ++i) {
			TempAllocator256 ta;
			DynamicString name(ta);
			sjson::parse_string(name, names[i]);
			DATA_COMPILER_ASSERT_RESOURCE_EXISTS(type, name.c_str(), opts);
			name += ".";
			name += type;
			opts.fake_read(name.c_str());

			ResourceOffset ro;
			ro.type = type_hash;
			ro.name = sjson::parse_resource_name(names[i]);
			hash_set::insert(output, ro);

			// Bring in requirements
			bring_in_requirements(output, opts, resource_id(ro.type, ro.name));
		}

		return 0;
	}

	s32 compile(CompileOptions &opts)
	{
		TempAllocator4096 ta;
		JsonObject obj(ta);
		JsonArray texture(ta);
		JsonArray script(ta);
		JsonArray sound(ta);
		JsonArray mesh(ta);
		JsonArray unit(ta);
		JsonArray sprite(ta);
		JsonArray material(ta);
		JsonArray font(ta);
		JsonArray level(ta);
		JsonArray phyconf(ta);
		JsonArray shader(ta);
		JsonArray sprite_animation(ta);

		Array<ResourceOffset> resources(default_allocator());
		HashSet<ResourceOffset> resources_set(default_allocator());

		Buffer buf = opts.read();
		sjson::parse(obj, buf);

		if (json_object::has(obj, "texture"))
			sjson::parse_array(texture, obj["texture"]);
		if (json_object::has(obj, "lua"))
			sjson::parse_array(script, obj["lua"]);
		if (json_object::has(obj, "sound"))
			sjson::parse_array(sound, obj["sound"]);
		if (json_object::has(obj, "mesh"))
			sjson::parse_array(mesh, obj["mesh"]);
		if (json_object::has(obj, "unit"))
			sjson::parse_array(unit, obj["unit"]);
		if (json_object::has(obj, "sprite"))
			sjson::parse_array(sprite, obj["sprite"]);
		if (json_object::has(obj, "material"))
			sjson::parse_array(material, obj["material"]);
		if (json_object::has(obj, "font"))
			sjson::parse_array(font, obj["font"]);
		if (json_object::has(obj, "level"))
			sjson::parse_array(level, obj["level"]);
		if (json_object::has(obj, "physics_config"))
			sjson::parse_array(phyconf, obj["physics_config"]);
		if (json_object::has(obj, "shader"))
			sjson::parse_array(shader, obj["shader"]);
		if (json_object::has(obj, "sprite_animation"))
			sjson::parse_array(sprite_animation, obj["sprite_animation"]);

		s32 err = 0;
		err = compile_resources(resources_set, opts, "texture", texture);
		DATA_COMPILER_ENSURE(err == 0, opts);
		err = compile_resources(resources_set, opts, "lua", script);
		DATA_COMPILER_ENSURE(err == 0, opts);
		err = compile_resources(resources_set, opts, "sound", sound);
		DATA_COMPILER_ENSURE(err == 0, opts);
		err = compile_resources(resources_set, opts, "mesh", mesh);
		DATA_COMPILER_ENSURE(err == 0, opts);
		err = compile_resources(resources_set, opts, "unit", unit);
		DATA_COMPILER_ENSURE(err == 0, opts);
		err = compile_resources(resources_set, opts, "sprite", sprite);
		DATA_COMPILER_ENSURE(err == 0, opts);
		err = compile_resources(resources_set, opts, "material", material);
		DATA_COMPILER_ENSURE(err == 0, opts);
		err = compile_resources(resources_set, opts, "font", font);
		DATA_COMPILER_ENSURE(err == 0, opts);
		err = compile_resources(resources_set, opts, "level", level);
		DATA_COMPILER_ENSURE(err == 0, opts);
		err = compile_resources(resources_set, opts, "physics_config", phyconf);
		DATA_COMPILER_ENSURE(err == 0, opts);
		err = compile_resources(resources_set, opts, "shader", shader);
		DATA_COMPILER_ENSURE(err == 0, opts);
		err = compile_resources(resources_set, opts, "sprite_animation", sprite_animation);
		DATA_COMPILER_ENSURE(err == 0, opts);

		// Generate resource list
		auto cur = hash_set::begin(resources_set);
		auto end = hash_set::end(resources_set);
		for (; cur != end; ++cur) {
			HASH_SET_SKIP_HOLE(resources_set, cur);

			array::push_back(resources, *cur);
		}

		// Write
		opts.write(RESOURCE_HEADER(RESOURCE_VERSION_PACKAGE));
		opts.write(array::size(resources));

		if (opts._bundle) {
			Buffer bundle_data(default_allocator());
			FileBuffer bundle_file(bundle_data);
			BinaryWriter bundle(bundle_file);

			for (u32 ii = 0; ii < array::size(resources); ++ii) {
				// Read the resource's compiled data.
				ResourceId id = resource_id(resources[ii].type, resources[ii].name);
				TempAllocator256 ta;
				DynamicString dest(ta);
				destination_path(dest, id);

				// Append data to bundle.
				File *data_file = opts._data_filesystem.open(dest.c_str(), FileOpenMode::READ);
				const u32 data_size = data_file->size();

				// Align data to a 16-bytes boundary.
				bundle.align(16);
				const u32 data_offset = array::size(bundle_data);

				file::copy(bundle_file, *data_file, data_size);
				opts._data_filesystem.close(*data_file);

				// Write ResourceOffset.
				opts.write(resources[ii].type);
				opts.write(resources[ii].name);
				opts.write(data_offset);
				opts.write(data_size);
			}

			// Write bundled data.
			opts.align(16);
			opts.write(array::begin(bundle_data), array::size(bundle_data));
		} else {
			for (u32 ii = 0; ii < array::size(resources); ++ii) {
				opts.write(resources[ii].type);
				opts.write(resources[ii].name);
				opts.write(UINT32_MAX);
				opts.write(UINT32_MAX);
			}
		}

		return 0;
	}

} // namespace package_resource_internal
#endif // if CROWN_CAN_COMPILE

namespace package_resource
{
	const ResourceOffset *resource_offset(const PackageResource *pr, u32 index)
	{
		const ResourceOffset *ro = (ResourceOffset *)(pr + 1);
		return ro + index;
	}

	const u8 *data(const PackageResource *pr)
	{
		const u8 *data_offset = (u8 *)resource_offset(pr, pr->num_resources);
		return (u8 *)memory::align_top(data_offset, 16);
	}

} // namespace package_resource

} // namespace crown
