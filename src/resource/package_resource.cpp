/*
 * Copyright (c) 2012-2026 Daniele Bartolini et al.
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
		, u32 *graph_level
		, ResourceId res_id
		, CompileOptions &opts
		)
	{
		const HashMap<DynamicString, u32> reqs_deffault(default_allocator());
		const HashMap<DynamicString, u32> &reqs = hash_map::get(opts._data_compiler._data_requirements, res_id, reqs_deffault);

		u32 max_graph_level = 0;
		u32 cur_graph_level;

		auto cur = hash_map::begin(reqs);
		auto end = hash_map::end(reqs);
		for (; cur != end; ++cur) {
			HASH_MAP_SKIP_HOLE(reqs, cur);

			const char *req_filename = cur->first.c_str();
			const char *req_type = resource_type(req_filename);
			const u32 req_name_len = resource_name_length(req_type, req_filename);

			const StringId64 req_type_hash(req_type);
			const StringId64 req_name_hash(req_filename, req_name_len);

			cur_graph_level = 0;
			bring_in_requirements(output, &cur_graph_level, resource_id(req_type_hash, req_name_hash), opts);
			max_graph_level = max(max_graph_level, cur_graph_level + 1);

			ResourceOffset ro;
			ro.type = req_type_hash;
			ro.name = req_name_hash;
			ro.online_order = cur_graph_level;
			ro._pad = 0;
			hash_set::insert(output, ro);
		}

		*graph_level = max_graph_level;
		return 0;
	}

	s32 compile_resources(HashSet<ResourceOffset> &output
		, const char *type
		, const JsonArray &names
		, CompileOptions &opts
		)
	{
		const StringId64 type_hash = StringId64(type);

		for (u32 i = 0; i < array::size(names); ++i) {
			TempAllocator256 ta;
			DynamicString name(ta);
			RETURN_IF_ERROR(sjson::parse_string(name, names[i]), opts);
			RETURN_IF_RESOURCE_MISSING(type, name.c_str(), opts);
			name += ".";
			name += type;
			opts.fake_read(name.c_str());

			ResourceOffset ro;
			ro.type = type_hash;
			ro.name = RETURN_IF_ERROR(sjson::parse_resource_name(names[i]), opts);

			// Bring in requirements
			u32 graph_level = 0;
			bring_in_requirements(output, &graph_level, resource_id(ro.type, ro.name), opts);

			ro.online_order = graph_level;
			ro._pad = 0;
			hash_set::insert(output, ro);
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
		JsonArray render_config(ta);

		Array<ResourceOffset> resources(default_allocator());
		HashSet<ResourceOffset> resources_set(default_allocator());

		Buffer buf = opts.read();
		RETURN_IF_ERROR(sjson::parse(obj, buf), opts);

		if (json_object::has(obj, "texture")) {
			RETURN_IF_ERROR(sjson::parse_array(texture, obj["texture"]), opts);
		}
		if (json_object::has(obj, "lua")) {
			RETURN_IF_ERROR(sjson::parse_array(script, obj["lua"]), opts);
		}
		if (json_object::has(obj, "sound")) {
			RETURN_IF_ERROR(sjson::parse_array(sound, obj["sound"]), opts);
		}
		if (json_object::has(obj, "mesh")) {
			RETURN_IF_ERROR(sjson::parse_array(mesh, obj["mesh"]), opts);
		}
		if (json_object::has(obj, "unit")) {
			RETURN_IF_ERROR(sjson::parse_array(unit, obj["unit"]), opts);
		}
		if (json_object::has(obj, "sprite")) {
			RETURN_IF_ERROR(sjson::parse_array(sprite, obj["sprite"]), opts);
		}
		if (json_object::has(obj, "material")) {
			RETURN_IF_ERROR(sjson::parse_array(material, obj["material"]), opts);
		}
		if (json_object::has(obj, "font")) {
			RETURN_IF_ERROR(sjson::parse_array(font, obj["font"]), opts);
		}
		if (json_object::has(obj, "level")) {
			RETURN_IF_ERROR(sjson::parse_array(level, obj["level"]), opts);
		}
		if (json_object::has(obj, "physics_config")) {
			RETURN_IF_ERROR(sjson::parse_array(phyconf, obj["physics_config"]), opts);
		}
		if (json_object::has(obj, "shader")) {
			RETURN_IF_ERROR(sjson::parse_array(shader, obj["shader"]), opts);
		}
		if (json_object::has(obj, "sprite_animation")) {
			RETURN_IF_ERROR(sjson::parse_array(sprite_animation, obj["sprite_animation"]), opts);
		}
		if (json_object::has(obj, "render_config")) {
			RETURN_IF_ERROR(sjson::parse_array(render_config, obj["render_config"]), opts);
		}

		s32 err = 0;
		err = compile_resources(resources_set, "texture", texture, opts);
		ENSURE_OR_RETURN(err == 0, opts);
		err = compile_resources(resources_set, "lua", script, opts);
		ENSURE_OR_RETURN(err == 0, opts);
		err = compile_resources(resources_set, "sound", sound, opts);
		ENSURE_OR_RETURN(err == 0, opts);
		err = compile_resources(resources_set, "mesh", mesh, opts);
		ENSURE_OR_RETURN(err == 0, opts);
		err = compile_resources(resources_set, "unit", unit, opts);
		ENSURE_OR_RETURN(err == 0, opts);
		err = compile_resources(resources_set, "sprite", sprite, opts);
		ENSURE_OR_RETURN(err == 0, opts);
		err = compile_resources(resources_set, "material", material, opts);
		ENSURE_OR_RETURN(err == 0, opts);
		err = compile_resources(resources_set, "font", font, opts);
		ENSURE_OR_RETURN(err == 0, opts);
		err = compile_resources(resources_set, "level", level, opts);
		ENSURE_OR_RETURN(err == 0, opts);
		err = compile_resources(resources_set, "physics_config", phyconf, opts);
		ENSURE_OR_RETURN(err == 0, opts);
		err = compile_resources(resources_set, "shader", shader, opts);
		ENSURE_OR_RETURN(err == 0, opts);
		err = compile_resources(resources_set, "sprite_animation", sprite_animation, opts);
		ENSURE_OR_RETURN(err == 0, opts);
		err = compile_resources(resources_set, "render_config", render_config, opts);
		ENSURE_OR_RETURN(err == 0, opts);

		// Generate resource list
		auto cur = hash_set::begin(resources_set);
		auto end = hash_set::end(resources_set);
		for (; cur != end; ++cur) {
			HASH_SET_SKIP_HOLE(resources_set, cur);

			array::push_back(resources, *cur);
		}

		// Generate sequential online order numbers.
		Array<ResourceOffset> original_resources = resources;
		u32 online_order = 0;
		for (u32 i = 0; i < array::size(resources); ++i) {
			for (u32 j = 0; j < array::size(resources); ++j) {
				if (original_resources[j].online_order == i)
					resources[j].online_order = online_order++;
			}
		}
		ENSURE_OR_RETURN(online_order == array::size(resources), opts);

		// Write
		opts.write(RESOURCE_HEADER(RESOURCE_VERSION_PACKAGE));
		opts.write(array::size(resources));

		if (opts._bundle) {
			Buffer bundle_data(default_allocator());
			FileBuffer bundle_file(bundle_data);
			BinaryWriter bundle(bundle_file);

			for (u32 ii = 0; ii < array::size(resources); ++ii) {
				ResourceId id = resource_id(resources[ii].type, resources[ii].name);

				{
					// Append data to bundle.
					TempAllocator256 ta;
					DynamicString dest(ta);
					destination_path(dest, id);

					File *data_file = opts._data_filesystem.open(dest.c_str(), FileOpenMode::READ);
					if (!data_file->is_open()) {
						opts._data_filesystem.close(*data_file);
						RETURN_IF_FALSE(false, opts, "Failed to open data");
					}

					// Align data to a 16-bytes boundary.
					bundle.align(16);
					const u32 data_offset = array::size(bundle_data);
					const u32 data_size = data_file->size();

					file::copy(bundle_file, *data_file, data_size);
					opts._data_filesystem.close(*data_file);

					// Write ResourceOffset.
					opts.write(resources[ii].type);
					opts.write(resources[ii].name);
					opts.write(data_offset);
					opts.write(data_size);
					opts.write(resources[ii].online_order);
					opts.write(resources[ii]._pad);
				}

				{
					// Copy stream data, if any, to bundle dir.
					TempAllocator256 ta;
					DynamicString stream_dest(ta);
					stream_destination_path(stream_dest, id);

					File *stream = opts._data_filesystem.open(stream_dest.c_str(), FileOpenMode::READ);
					if (stream->is_open()) {
						File *bundle_stream = opts._output_filesystem.open(stream_dest.c_str(), FileOpenMode::WRITE);
						RETURN_IF_FALSE(bundle_stream->is_open(), opts, "Failed to open bundle stream");
						file::copy(*bundle_stream, *stream, stream->size());
						opts._output_filesystem.close(*bundle_stream);
					}
					opts._data_filesystem.close(*stream);
				}
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
				opts.write(resources[ii].online_order);
				opts.write(resources[ii]._pad);
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
