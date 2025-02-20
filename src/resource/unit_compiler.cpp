/*
 * Copyright (c) 2012-2025 Daniele Bartolini et al.
 * SPDX-License-Identifier: MIT
 */

#include "config.h"

#if CROWN_CAN_COMPILE
#include "core/containers/array.inl"
#include "core/containers/hash_map.inl"
#include "core/filesystem/file_buffer.inl"
#include "core/guid.inl"
#include "core/json/json_object.inl"
#include "core/json/sjson.h"
#include "core/math/math.h"
#include "core/memory/temp_allocator.inl"
#include "core/strings/dynamic_string.inl"
#include "core/strings/string_id.inl"
#include "resource/compile_options.inl"
#include "resource/physics_resource.h"
#include "resource/resource_id.inl"
#include "resource/unit_compiler.h"
#include "resource/unit_resource.h"
#include "world/types.h"
#include <algorithm>

namespace crown
{
struct ProjectionInfo
{
	const char *name;
	ProjectionType::Enum type;
};

static const ProjectionInfo s_projection[] =
{
	{ "perspective",  ProjectionType::PERSPECTIVE  },
	{ "orthographic", ProjectionType::ORTHOGRAPHIC }
};
CE_STATIC_ASSERT(countof(s_projection) == ProjectionType::COUNT);

struct LightInfo
{
	const char *name;
	LightType::Enum type;
};

static const LightInfo s_light[] =
{
	{ "directional", LightType::DIRECTIONAL },
	{ "omni",        LightType::OMNI        },
	{ "spot",        LightType::SPOT        }
};
CE_STATIC_ASSERT(countof(s_light) == LightType::COUNT);

static ProjectionType::Enum projection_name_to_enum(const char *name)
{
	for (u32 i = 0; i < countof(s_projection); ++i) {
		if (strcmp(name, s_projection[i].name) == 0)
			return s_projection[i].type;
	}

	return ProjectionType::COUNT;
}

static LightType::Enum light_name_to_enum(const char *name)
{
	for (u32 i = 0; i < countof(s_light); ++i) {
		if (strcmp(name, s_light[i].name) == 0)
			return s_light[i].type;
	}

	return LightType::COUNT;
}

static s32 compile_transform(Buffer &output, const char *json, CompileOptions &opts)
{
	TempAllocator4096 ta;
	JsonObject obj(ta);
	RETURN_IF_ERROR(sjson::parse(obj, json), opts);

	TransformDesc td;
	td.position = RETURN_IF_ERROR(sjson::parse_vector3   (obj["position"]), opts);
	td.rotation = RETURN_IF_ERROR(sjson::parse_quaternion(obj["rotation"]), opts);
	td.scale    = RETURN_IF_ERROR(sjson::parse_vector3   (obj["scale"]), opts);

	FileBuffer fb(output);
	BinaryWriter bw(fb);
	bw.write(td.position);
	bw.write(td.rotation);
	bw.write(td.scale);
	return 0;
}

static s32 compile_camera(Buffer &output, const char *json, CompileOptions &opts)
{
	TempAllocator4096 ta;
	JsonObject obj(ta);
	RETURN_IF_ERROR(sjson::parse(obj, json), opts);

	DynamicString type(ta);
	RETURN_IF_ERROR(sjson::parse_string(type, obj["projection"]), opts);

	ProjectionType::Enum pt = projection_name_to_enum(type.c_str());
	RETURN_IF_FALSE(pt != ProjectionType::COUNT
		, opts
		, "Unknown projection type: '%s'"
		, type.c_str()
		);

	CameraDesc cd;
	cd.type       = pt;
	cd.fov        = RETURN_IF_ERROR(sjson::parse_float(obj["fov"]), opts);
	cd.near_range = RETURN_IF_ERROR(sjson::parse_float(obj["near_range"]), opts);
	cd.far_range  = RETURN_IF_ERROR(sjson::parse_float(obj["far_range"]), opts);

	FileBuffer fb(output);
	BinaryWriter bw(fb);
	bw.write(cd.type);
	bw.write(cd.fov);
	bw.write(cd.near_range);
	bw.write(cd.far_range);
	return 0;
}

static s32 compile_mesh_renderer(Buffer &output, const char *json, CompileOptions &opts)
{
	TempAllocator4096 ta;
	JsonObject obj(ta);
	RETURN_IF_ERROR(sjson::parse(obj, json), opts);

	DynamicString mesh_resource(ta);
	RETURN_IF_ERROR(sjson::parse_string(mesh_resource, obj["mesh_resource"]), opts);

	RETURN_IF_RESOURCE_MISSING("mesh", mesh_resource.c_str(), opts);
	opts.add_requirement("mesh", mesh_resource.c_str());

	DynamicString material(ta);
	RETURN_IF_ERROR(sjson::parse_string(material, obj["material"]), opts);
	RETURN_IF_RESOURCE_MISSING("material"
		, material.c_str()
		, opts
		);
	opts.add_requirement("material", material.c_str());

	MeshRendererDesc mrd;
	mrd.mesh_resource     = RETURN_IF_ERROR(sjson::parse_resource_name(obj["mesh_resource"]), opts);
	mrd.material_resource = RETURN_IF_ERROR(sjson::parse_resource_name(obj["material"]), opts);
	mrd.geometry_name     = RETURN_IF_ERROR(sjson::parse_string_id    (obj["geometry_name"]), opts);
	mrd.visible           = RETURN_IF_ERROR(sjson::parse_bool         (obj["visible"]), opts);
	mrd._pad0[0]          = 0;
	mrd._pad0[1]          = 0;
	mrd._pad0[2]          = 0;

	FileBuffer fb(output);
	BinaryWriter bw(fb);
	bw.write(mrd.mesh_resource);
	bw.write(mrd.material_resource);
	bw.write(mrd.geometry_name);
	bw.write(mrd.visible);
	bw.write(mrd._pad0[0]);
	bw.write(mrd._pad0[1]);
	bw.write(mrd._pad0[2]);
	return 0;
}

static s32 compile_sprite_renderer(Buffer &output, const char *json, CompileOptions &opts)
{
	TempAllocator4096 ta;
	JsonObject obj(ta);
	RETURN_IF_ERROR(sjson::parse(obj, json), opts);

	DynamicString sprite_resource(ta);
	RETURN_IF_ERROR(sjson::parse_string(sprite_resource, obj["sprite_resource"]), opts);
	RETURN_IF_RESOURCE_MISSING("sprite"
		, sprite_resource.c_str()
		, opts
		);
	opts.add_requirement("sprite", sprite_resource.c_str());

	DynamicString material(ta);
	RETURN_IF_ERROR(sjson::parse_string(material, obj["material"]), opts);
	RETURN_IF_RESOURCE_MISSING("material"
		, material.c_str()
		, opts
		);
	opts.add_requirement("material", material.c_str());

	SpriteRendererDesc srd;
	srd.sprite_resource   = RETURN_IF_ERROR(sjson::parse_resource_name(obj["sprite_resource"]), opts);
	srd.material_resource = RETURN_IF_ERROR(sjson::parse_resource_name(obj["material"]), opts);
	srd.layer             = RETURN_IF_ERROR(sjson::parse_int          (obj["layer"]), opts);
	srd.depth             = RETURN_IF_ERROR(sjson::parse_int          (obj["depth"]), opts);
	srd.visible           = RETURN_IF_ERROR(sjson::parse_bool         (obj["visible"]), opts);
	srd._pad0[0]          = 0;
	srd._pad0[1]          = 0;
	srd._pad0[2]          = 0;
	srd._pad1[0]          = 0;
	srd._pad1[1]          = 0;
	srd._pad1[2]          = 0;
	srd._pad1[3]          = 0;

	FileBuffer fb(output);
	BinaryWriter bw(fb);
	bw.write(srd.sprite_resource);
	bw.write(srd.material_resource);
	bw.write(srd.layer);
	bw.write(srd.depth);
	bw.write(srd.visible);
	bw.write(srd._pad0[0]);
	bw.write(srd._pad0[1]);
	bw.write(srd._pad0[2]);
	bw.write(srd._pad1[0]);
	bw.write(srd._pad1[1]);
	bw.write(srd._pad1[2]);
	bw.write(srd._pad1[3]);
	return 0;
}

static s32 compile_light(Buffer &output, const char *json, CompileOptions &opts)
{
	TempAllocator4096 ta;
	JsonObject obj(ta);
	RETURN_IF_ERROR(sjson::parse(obj, json), opts);

	DynamicString type(ta);
	RETURN_IF_ERROR(sjson::parse_string(type, obj["type"]), opts);

	LightType::Enum lt = light_name_to_enum(type.c_str());
	RETURN_IF_FALSE(lt != LightType::COUNT
		, opts
		, "Unknown light type: '%s'"
		, type.c_str()
		);

	LightDesc ld;
	ld.type       = lt;
	ld.range      = RETURN_IF_ERROR(sjson::parse_float  (obj["range"]), opts);
	ld.intensity  = RETURN_IF_ERROR(sjson::parse_float  (obj["intensity"]), opts);
	ld.spot_angle = RETURN_IF_ERROR(sjson::parse_float  (obj["spot_angle"]), opts);
	ld.color      = RETURN_IF_ERROR(sjson::parse_vector3(obj["color"]), opts);

	FileBuffer fb(output);
	BinaryWriter bw(fb);
	bw.write(ld.type);
	bw.write(ld.range);
	bw.write(ld.intensity);
	bw.write(ld.spot_angle);
	bw.write(ld.color);
	return 0;
}

static s32 compile_script(Buffer &output, const char *json, CompileOptions &opts)
{
	TempAllocator4096 ta;
	JsonObject obj(ta);
	RETURN_IF_ERROR(sjson::parse(obj, json), opts);

	DynamicString script_resource(ta);
	RETURN_IF_ERROR(sjson::parse_string(script_resource, obj["script_resource"]), opts);
	RETURN_IF_RESOURCE_MISSING("lua"
		, script_resource.c_str()
		, opts
		);
	opts.add_requirement("lua", script_resource.c_str());

	ScriptDesc sd;
	sd.script_resource = RETURN_IF_ERROR(sjson::parse_resource_name(obj["script_resource"]), opts);

	FileBuffer fb(output);
	BinaryWriter bw(fb);
	bw.write(sd.script_resource);
	return 0;
}

static s32 compile_animation_state_machine(Buffer &output, const char *json, CompileOptions &opts)
{
	TempAllocator4096 ta;
	JsonObject obj(ta);
	RETURN_IF_ERROR(sjson::parse(obj, json), opts);

	DynamicString state_machine_resource(ta);
	RETURN_IF_ERROR(sjson::parse_string(state_machine_resource, obj["state_machine_resource"]), opts);
	RETURN_IF_RESOURCE_MISSING("state_machine"
		, state_machine_resource.c_str()
		, opts
		);
	opts.add_requirement("state_machine", state_machine_resource.c_str());

	AnimationStateMachineDesc asmd;
	asmd.state_machine_resource = RETURN_IF_ERROR(sjson::parse_resource_name(obj["state_machine_resource"]), opts);

	FileBuffer fb(output);
	BinaryWriter bw(fb);
	bw.write(asmd.state_machine_resource);
	return 0;
}

namespace unit_compiler
{
	Buffer read_unit(UnitCompiler &c, const char *path)
	{
		Buffer buf = c._opts.read(path);
		array::push_back(buf, '\0');
		return buf;
	}

	// Collects the prefabs data of @a unit_json and its children recursively.
	// Prefab of deeper child is collected first.
	//
	// Consider the following hierarchy:
	//
	//   Unit      Prefab
	//   A         P
	//   +-B
	//   +-C       Q
	//      +-D    +-R
	//
	// collect_prefab(A) will collect the data needed to compile
	// the unit 'A' and all its children:
	//
	// prefab_offsets[  0] = P = { prefab = nil }   <- Prefab of A
	// prefab_offsets[  1] = Q = { prefab = nil }   <- Prefab of C
	// prefab_offsets[n-1] = A = { prefab = P   }   <- A itself
	s32 collect_prefabs(UnitCompiler &c, StringId64 unit_name, const char *unit_json, bool append_data)
	{
		TempAllocator4096 ta;
		JsonObject prefab(ta);
		RETURN_IF_ERROR(sjson::parse(prefab, unit_json), c._opts);

		if (json_object::has(prefab, "children")) {
			JsonArray children(ta);
			RETURN_IF_ERROR(sjson::parse_array(children, prefab["children"]), c._opts);

			for (u32 i = 0; i < array::size(children); ++i) {
				s32 err = collect_prefabs(c, unit_name, children[i], false);
				ENSURE_OR_RETURN(err == 0, c._opts);
			}
		}

		if (json_object::has(prefab, "prefab")) {
			TempAllocator512 ta;
			DynamicString path(ta);
			RETURN_IF_ERROR(sjson::parse_string(path, prefab["prefab"]), c._opts);
			RETURN_IF_RESOURCE_MISSING("unit"
				, path.c_str()
				, c._opts
				);
			StringId64 name(path.c_str());
			path += ".unit";

			Buffer buf = read_unit(c, path.c_str());
			s32 err = collect_prefabs(c, name, array::begin(buf), true);
			ENSURE_OR_RETURN(err == 0, c._opts);
		}

		if (append_data) {
			u32 prefab_offset = array::size(c._prefab_data);
			array::push(c._prefab_data, unit_json, strlen32(unit_json));
			array::push_back(c._prefab_data, '\0');
			array::push_back(c._prefab_offsets, prefab_offset);
			array::push_back(c._prefab_names, unit_name);
		}

		return 0;
	}

	const char *prefab_json(UnitCompiler &c, const char *prefab_name)
	{
		StringId64 name(prefab_name);
		for (u32 i = 0; i < array::size(c._prefab_names); ++i) {
			if (c._prefab_names[i] == name)
				return &c._prefab_data[c._prefab_offsets[i]];
		}

		return NULL;
	}

	u32 object_index(const JsonArray &objects, const Guid &object_id, CompileOptions &opts)
	{
		for (u32 i = 0; i < array::size(objects); ++i) {
			TempAllocator512 ta;
			JsonObject obj(ta);
			RETURN_IF_ERROR(sjson::parse(obj, objects[i]), opts);

			if (json_object::has(obj, "id")) {
				Guid id = RETURN_IF_ERROR(sjson::parse_guid(obj["id"]), opts);
				if (id == object_id)
					return i;
			} else {
				Guid id = RETURN_IF_ERROR(sjson::parse_guid(obj["_guid"]), opts);
				if (id == object_id)
					return i;
			}
		}

		return UINT32_MAX;
	}

	Unit *find_children(Unit *unit, Guid id)
	{
		CE_ENSURE(unit != NULL);

		auto cur = hash_map::begin(unit->_children);
		auto end = hash_map::end(unit->_children);
		for (; cur != end; ++cur) {
			HASH_MAP_SKIP_HOLE(unit->_children, cur);

			if (cur->first == id)
				return cur->second;

			Unit *child = find_children(cur->second, id);
			if (child != NULL)
				return child;
		}

		return NULL;
	}

	void delete_unit(Unit *unit)
	{
		auto cur = hash_map::begin(unit->_children);
		auto end = hash_map::end(unit->_children);
		for (; cur != end; ++cur) {
			HASH_MAP_SKIP_HOLE(unit->_children, cur);

			delete_unit(cur->second);
		}

		CE_DELETE(default_allocator(), unit);
	}

	s32 modify_unit_components(UnitCompiler &c, Unit *unit, const char *unit_json)
	{
		TempAllocator4096 ta;
		JsonObject obj(ta);
		RETURN_IF_ERROR(sjson::parse(obj, unit_json), c._opts);

		if (json_object::has(obj, "components")) {
			JsonArray components(ta);
			RETURN_IF_ERROR(sjson::parse_array(components, obj["components"]), c._opts);

			// Add components.
			for (u32 cc = 0; cc < array::size(components); ++cc) {
				JsonObject component(ta);
				RETURN_IF_ERROR(sjson::parse_object(component, components[cc]), c._opts);

				array::push_back(unit->_merged_components, components[cc]);
				array::push_back(unit->_merged_components_data, component["data"]);
			}
		}

		if (json_object::has(obj, "deleted_components")) {
			JsonObject deleted_components(ta);
			RETURN_IF_ERROR(sjson::parse_object(deleted_components, obj["deleted_components"]), c._opts);

			// Delete components.
			auto cur = json_object::begin(deleted_components);
			auto end = json_object::end(deleted_components);
			for (; cur != end; ++cur) {
				JSON_OBJECT_SKIP_HOLE(deleted_components, cur);

				auto key = cur->first;

				// Extract GUID from key "#xxxxxxxx-xxxx-xxxx-xxxx-xxxxxxxxxxxx".
				char guid[37];
				strncpy(guid, key.data() + 1, sizeof(guid) - 1);
				guid[36] = '\0';
				Guid component_id = guid::parse(guid);

				u32 comp_idx = object_index(unit->_merged_components, component_id, c._opts);
				if (comp_idx != UINT32_MAX) {
					u32 comp_last = array::size(unit->_merged_components) - 1;
					unit->_merged_components[comp_idx] = unit->_merged_components[comp_last];
					array::pop_back(unit->_merged_components);
					unit->_merged_components_data[comp_idx] = unit->_merged_components_data[comp_last];
					array::pop_back(unit->_merged_components_data);
				} else {
					char buf[GUID_BUF_LEN];
					RETURN_IF_FALSE(false
						, c._opts
						, "Deletion of unexisting component ID: %s"
						, guid::to_string(buf, sizeof(buf), component_id)
						);
				}
			}
		}

		if (json_object::has(obj, "modified_components")) {
			JsonObject modified_components(ta);
			RETURN_IF_ERROR(sjson::parse(modified_components, obj["modified_components"]), c._opts);

			// Modify components.
			auto cur = json_object::begin(modified_components);
			auto end = json_object::end(modified_components);
			for (; cur != end; ++cur) {
				JSON_OBJECT_SKIP_HOLE(modified_components, cur);

				auto key = cur->first;
				auto val = cur->second;

				// Extract GUID from key "#xxxxxxxx-xxxx-xxxx-xxxx-xxxxxxxxxxxx".
				char guid[37];
				strncpy(guid, key.data() + 1, sizeof(guid) - 1);
				guid[36] = '\0';
				Guid component_id = guid::parse(guid);

				// Patch component "data" key.
				u32 comp_idx = object_index(unit->_merged_components, component_id, c._opts);
				if (comp_idx != UINT32_MAX) {
					JsonObject modified_component(ta);
					RETURN_IF_ERROR(sjson::parse_object(modified_component, val), c._opts);

					unit->_merged_components_data[comp_idx] = modified_component["data"];
				} else {
					char buf[GUID_BUF_LEN];
					RETURN_IF_FALSE(false
						, c._opts
						, "Modification of unexisting component ID: %s"
						, guid::to_string(buf, sizeof(buf), component_id)
						);
				}
			}
		}

		return 0;
	}

	s32 parse_unit_internal(UnitCompiler &c
		, const char *unit_json
		, Unit *instance_unit
		, Unit *parent_unit
		)
	{
		TempAllocator4096 ta;
		JsonObject obj(ta);
		RETURN_IF_ERROR(sjson::parse(obj, unit_json), c._opts);

		Guid id = RETURN_IF_ERROR(sjson::parse_guid(obj["_guid"]), c._opts);

		Unit *unit = instance_unit;
		if (unit == NULL) {
			unit = CE_NEW(default_allocator(), Unit)(default_allocator());
		}

		if (instance_unit == NULL) {
			if (parent_unit == NULL) {
				hash_map::set(c._units, id, unit);
			} else {
				unit->_parent = parent_unit;
				hash_map::set(parent_unit->_children, id, unit);
			}
		}

		if (json_object::has(obj, "prefab")) {
			TempAllocator512 ta;
			DynamicString prefab(ta);
			RETURN_IF_ERROR(sjson::parse_string(prefab, obj["prefab"]), c._opts);
			const char *prefab_json_data = prefab_json(c, prefab.c_str());
			RETURN_IF_FALSE(prefab_json_data != NULL
				, c._opts
				, "Unknown prefab: '%s'"
				, prefab.c_str()
				);

			s32 err = parse_unit_internal(c
				, prefab_json_data
				, unit
				, NULL
				);
			ENSURE_OR_RETURN(err == 0, c._opts);
		}

		s32 err = modify_unit_components(c, unit, unit_json);
		ENSURE_OR_RETURN(err == 0, c._opts);

		if (json_object::has(obj, "children")) {
			JsonArray children(ta);
			RETURN_IF_ERROR(sjson::parse_array(children, obj["children"]), c._opts);

			for (u32 cc = 0; cc < array::size(children); ++cc) {
				s32 err = parse_unit_internal(c
					, children[cc]
					, NULL
					, unit
					);
				ENSURE_OR_RETURN(err == 0, c._opts);
			}
		}

		if (json_object::has(obj, "deleted_children")) {
			JsonArray deleted_children(ta);
			RETURN_IF_ERROR(sjson::parse_array(deleted_children, obj["deleted_children"]), c._opts);

			// Delete children.
			for (u32 ii = 0; ii < array::size(deleted_children); ++ii) {
				JsonObject obj(ta);
				RETURN_IF_ERROR(sjson::parse_object(obj, deleted_children[ii]), c._opts);
				Guid id = RETURN_IF_ERROR(sjson::parse_guid(obj["id"]), c._opts);

				Unit *child = find_children(unit, id);

				char buf[GUID_BUF_LEN];
				RETURN_IF_FALSE(child != NULL
					, c._opts
					, "Deletion of unexisting child ID: %s"
					, guid::to_string(buf, sizeof(buf), id)
					);

				Unit *child_parent = child->_parent;
				delete_unit(child);
				hash_map::remove(child_parent->_children, id);
			}
		}

		if (json_object::has(obj, "modified_children")) {
			JsonArray modified_children(ta);
			RETURN_IF_ERROR(sjson::parse_array(modified_children, obj["modified_children"]), c._opts);

			for (u32 ii = 0; ii < array::size(modified_children); ++ii) {
				JsonObject obj(ta);
				RETURN_IF_ERROR(sjson::parse_object(obj, modified_children[ii]), c._opts);
				Guid id = RETURN_IF_ERROR(sjson::parse_guid(obj["id"]), c._opts);

				Unit *child = find_children(unit, id);

				char buf[GUID_BUF_LEN];
				RETURN_IF_FALSE(child != NULL
					, c._opts
					, "Modification of unexisting child ID: %s"
					, guid::to_string(buf, sizeof(buf), id)
					);

				s32 err = modify_unit_components(c, child, modified_children[ii]);
				ENSURE_OR_RETURN(err == 0, c._opts);
			}
		}

		// Parse unit's editor name.
		if (json_object::has(obj, "editor")) {
			JsonObject editor(ta);
			RETURN_IF_ERROR(sjson::parse(editor, obj["editor"]), c._opts);

			if (json_object::has(editor, "name")) {
				unit->_editor_name = RETURN_IF_ERROR(sjson::parse_string_id(editor["name"]), c._opts);
			}
		}

		return 0;
	}

	s32 parse_unit_from_json(UnitCompiler &c, const char *unit_json)
	{
		s32 err = collect_prefabs(c, StringId64(), unit_json, true);
		ENSURE_OR_RETURN(err == 0, c._opts);

		u32 original_unit = c._prefab_offsets[array::size(c._prefab_offsets) - 1];
		return parse_unit_internal(c, &c._prefab_data[original_unit], NULL, NULL);
	}

	s32 parse_unit(UnitCompiler &c, const char *path)
	{
		return parse_unit_from_json(c, array::begin(read_unit(c, path)));
	}

	s32 parse_unit_array_from_json(UnitCompiler &c, const char *units_array_json)
	{
		TempAllocator4096 ta;
		JsonArray units(ta);
		RETURN_IF_ERROR(sjson::parse_array(units, units_array_json), c._opts);

		Array<u32> original_units(default_allocator());

		for (u32 i = 0; i < array::size(units); ++i) {
			s32 err = collect_prefabs(c, StringId64(), units[i], true);
			ENSURE_OR_RETURN(err == 0, c._opts);
			u32 original_unit = c._prefab_offsets[array::size(c._prefab_offsets) - 1];
			array::push_back(original_units, original_unit);
		}

		for (u32 i = 0; i < array::size(units); ++i) {
			s32 err = parse_unit_internal(c, &c._prefab_data[original_units[i]], NULL, NULL);
			ENSURE_OR_RETURN(err == 0, c._opts);
		}

		return 0;
	}

	s32 flatten_unit(UnitCompiler &c, Unit *unit, u32 parent_unit_index)
	{
		const u32 unit_index = c._num_units;
		bool unit_has_transform = false;

		// Compile component data for each component type found
		// in the tree of units.
		for (u32 cc = 0; cc < array::size(unit->_merged_components); ++cc) {
			const char *component_json = unit->_merged_components[cc];

			TempAllocator512 ta;
			JsonObject component(ta);
			RETURN_IF_ERROR(sjson::parse(component, component_json), c._opts);

			StringId32 comp_type;
			if (json_object::has(component, "type")) {
				comp_type = RETURN_IF_ERROR(sjson::parse_string_id(component["type"]), c._opts);
			} else {
				comp_type = RETURN_IF_ERROR(sjson::parse_string_id(component["_type"]), c._opts);
			}

			if (comp_type == STRING_ID_32("transform", UINT32_C(0xad9b5315)))
				unit_has_transform = true;

			// Append data to the component data for the given type.
			ComponentTypeData ctd_deffault(default_allocator());
			ComponentTypeData &ctd = const_cast<ComponentTypeData &>(hash_map::get(c._component_data, comp_type, ctd_deffault));
			RETURN_IF_FALSE(&ctd != &ctd_deffault, c._opts, "Unknown component type");

			// Compile component.
			Buffer comp_data(default_allocator());
			s32 err = ctd._compiler(comp_data, unit->_merged_components_data[cc], c._opts);
			ENSURE_OR_RETURN(err == 0, c._opts);

			// One component per unit max.
			auto cur = array::begin(ctd._unit_index);
			auto end = array::end(ctd._unit_index);
			if (std::find(cur, end, unit_index) != end) {
				char buf[STRING_ID32_BUF_LEN];
				RETURN_IF_FALSE(false
					, c._opts
					, "Unit already has a component of type: %s"
					, comp_type.to_string(buf, sizeof(buf))
					);
			}

			array::push(ctd._data, array::begin(comp_data), array::size(comp_data));
			array::push_back(ctd._unit_index, unit_index);
			++ctd._num;
		}

		array::push_back(c._unit_parents, parent_unit_index);
		array::push_back(c._unit_names, unit->_editor_name);
		++c._num_units;

		// Flatten children tree.
		auto cur = hash_map::begin(unit->_children);
		auto end = hash_map::end(unit->_children);
		for (; cur != end; ++cur) {
			HASH_MAP_SKIP_HOLE(unit->_children, cur);

			RETURN_IF_FALSE(unit_has_transform
				, c._opts
				, "Units with children must have 'transform' component"
				);
			s32 err = flatten_unit(c, cur->second, unit_index);
			ENSURE_OR_RETURN(err == 0, c._opts);
		}

		return 0;
	}

	void register_component_compiler(UnitCompiler &c, StringId32 type, CompileFunction fn, f32 spawn_order)
	{
		ComponentTypeData ctd(default_allocator());
		ctd._compiler = fn;

		ComponentTypeInfo cti;
		cti._type = type;
		cti._spawn_order = spawn_order;

		hash_map::set(c._component_data, type, ctd);

		array::push_back(c._component_info, cti);
		std::sort(array::begin(c._component_info), array::end(c._component_info));
	}

	void register_component_compiler(UnitCompiler &c, const char *type, CompileFunction fn, f32 spawn_order)
	{
		register_component_compiler(c, StringId32(type), fn, spawn_order);
	}

	s32 flatten(UnitCompiler &c)
	{
		auto cur = hash_map::begin(c._units);
		auto end = hash_map::end(c._units);
		for (; cur != end; ++cur) {
			HASH_MAP_SKIP_HOLE(c._units, cur);

			s32 err = flatten_unit(c, cur->second, UINT32_MAX);
			ENSURE_OR_RETURN(err == 0, c._opts);
		}

		return 0;
	}

	s32 blob(Buffer &output, UnitCompiler &c)
	{
		FileBuffer fb(output);
		BinaryWriter bw(fb);

		s32 err = flatten(c);
		ENSURE_OR_RETURN(err == 0, c._opts);

		// Count component types.
		u32 num_component_types = 0;
		auto cur = hash_map::begin(c._component_data);
		auto end = hash_map::end(c._component_data);
		for (; cur != end; ++cur) {
			HASH_MAP_SKIP_HOLE(c._component_data, cur);

			if (cur->second._num > 0)
				++num_component_types;
		}

		// Write header.
		UnitResource ur;
		ur.version = RESOURCE_HEADER(RESOURCE_VERSION_UNIT);
		ur.num_units = c._num_units;
		ur.num_component_types = num_component_types;

		bw.write(ur.version);
		bw.write(ur.num_units);
		bw.write(ur.num_component_types);

		// Write parents.
		for (u32 ii = 0; ii < c._num_units; ++ii)
			bw.write(c._unit_parents[ii]);

		for (u32 ii = 0; ii < array::size(c._component_info); ++ii) {
			const StringId32 comp_type = c._component_info[ii]._type;

			const ComponentTypeData ctd_deffault(default_allocator());
			const ComponentTypeData &ctd = hash_map::get(c._component_data, comp_type, ctd_deffault);

			if (ctd._num == 0)
				continue;

			// Write component data.
			ComponentData cd;
			cd.type = comp_type;
			cd.num_instances = ctd._num;
			cd.data_size = array::size(ctd._data);

			bw.align(alignof(cd));
			bw.write(cd.type);
			bw.write(cd.num_instances);
			bw.write(cd.data_size);
			for (u32 jj = 0; jj < array::size(ctd._unit_index); ++jj)
				bw.write(ctd._unit_index[jj]);
			bw.align(16);
			bw.write(array::begin(ctd._data), array::size(ctd._data));
		}

		return 0;
	}

} // namespace unit_compiler

Unit::Unit(Allocator &a)
	: _merged_components(a)
	, _merged_components_data(a)
	, _children(a)
	, _parent(NULL)
{
}

UnitCompiler::UnitCompiler(Allocator &a, CompileOptions &opts)
	: _units(a)
	, _opts(opts)
	, _prefab_data(a)
	, _prefab_offsets(a)
	, _prefab_names(a)
	, _component_data(a)
	, _component_info(a)
	, _unit_names(a)
	, _unit_parents(a)
	, _num_units(0)
{
	unit_compiler::register_component_compiler(*this, "transform",               &compile_transform,                           0.0f);
	unit_compiler::register_component_compiler(*this, "camera",                  &compile_camera,                              1.0f);
	unit_compiler::register_component_compiler(*this, "mesh_renderer",           &compile_mesh_renderer,                       1.0f);
	unit_compiler::register_component_compiler(*this, "sprite_renderer",         &compile_sprite_renderer,                     1.0f);
	unit_compiler::register_component_compiler(*this, "light",                   &compile_light,                               1.0f);
	unit_compiler::register_component_compiler(*this, "script",                  &compile_script,                              1.0f);
	unit_compiler::register_component_compiler(*this, "collider",                &physics_resource_internal::compile_collider, 1.0f);
	unit_compiler::register_component_compiler(*this, "actor",                   &physics_resource_internal::compile_actor,    2.0f);
	unit_compiler::register_component_compiler(*this, "joint",                   &physics_resource_internal::compile_joint,    3.0f);
	unit_compiler::register_component_compiler(*this, "animation_state_machine", &compile_animation_state_machine,             1.0f);
}

UnitCompiler::~UnitCompiler()
{
	auto cur = hash_map::begin(_units);
	auto end = hash_map::end(_units);
	for (; cur != end; ++cur) {
		HASH_MAP_SKIP_HOLE(_units, cur);

		unit_compiler::delete_unit(cur->second);
	}

	hash_map::clear(_units);
}

} // namespace crown

#endif // if CROWN_CAN_COMPILE
