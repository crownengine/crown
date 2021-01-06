/*
 * Copyright (c) 2012-2021 Daniele Bartolini et al.
 * License: https://github.com/dbartolini/crown/blob/master/LICENSE
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
#include "resource/unit_compiler.h"
#include "resource/unit_resource.h"
#include "world/types.h"
#include <algorithm>

namespace crown
{
struct ProjectionInfo
{
	const char* name;
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
	const char* name;
	LightType::Enum type;
};

static const LightInfo s_light[] =
{
	{ "directional", LightType::DIRECTIONAL },
	{ "omni",        LightType::OMNI        },
	{ "spot",        LightType::SPOT        }
};
CE_STATIC_ASSERT(countof(s_light) == LightType::COUNT);

static ProjectionType::Enum projection_name_to_enum(const char* name)
{
	for (u32 i = 0; i < countof(s_projection); ++i)
	{
		if (strcmp(name, s_projection[i].name) == 0)
			return s_projection[i].type;
	}

	return ProjectionType::COUNT;
}

static LightType::Enum light_name_to_enum(const char* name)
{
	for (u32 i = 0; i < countof(s_light); ++i)
	{
		if (strcmp(name, s_light[i].name) == 0)
			return s_light[i].type;
	}

	return LightType::COUNT;
}

static s32 compile_transform(Buffer& output, const char* json, CompileOptions& /*opts*/)
{
	TempAllocator4096 ta;
	JsonObject obj(ta);
	sjson::parse(obj, json);

	TransformDesc td;
	td.position = sjson::parse_vector3   (obj["position"]);
	td.rotation = sjson::parse_quaternion(obj["rotation"]);
	td.scale    = sjson::parse_vector3   (obj["scale"]);

	FileBuffer fb(output);
	BinaryWriter bw(fb);
	bw.write(td.position);
	bw.write(td.rotation);
	bw.write(td.scale);
	return 0;
}

static s32 compile_camera(Buffer& output, const char* json, CompileOptions& opts)
{
	TempAllocator4096 ta;
	JsonObject obj(ta);
	sjson::parse(obj, json);

	DynamicString type(ta);
	sjson::parse_string(type, obj["projection"]);

	ProjectionType::Enum pt = projection_name_to_enum(type.c_str());
	DATA_COMPILER_ASSERT(pt != ProjectionType::COUNT
		, opts
		, "Unknown projection type: '%s'"
		, type.c_str()
		);

	CameraDesc cd;
	cd.type       = pt;
	cd.fov        = sjson::parse_float(obj["fov"]);
	cd.near_range = sjson::parse_float(obj["near_range"]);
	cd.far_range  = sjson::parse_float(obj["far_range"]);

	FileBuffer fb(output);
	BinaryWriter bw(fb);
	bw.write(cd.type);
	bw.write(cd.fov);
	bw.write(cd.near_range);
	bw.write(cd.far_range);
	return 0;
}

static s32 compile_mesh_renderer(Buffer& output, const char* json, CompileOptions& opts)
{
	TempAllocator4096 ta;
	JsonObject obj(ta);
	sjson::parse(obj, json);

	DynamicString mesh_resource(ta);
	sjson::parse_string(mesh_resource, obj["mesh_resource"]);
	DATA_COMPILER_ASSERT_RESOURCE_EXISTS("mesh"
		, mesh_resource.c_str()
		, opts
		);
	opts.add_requirement("mesh", mesh_resource.c_str());

	DynamicString material(ta);
	sjson::parse_string(material, obj["material"]);
	DATA_COMPILER_ASSERT_RESOURCE_EXISTS("material"
		, material.c_str()
		, opts
		);
	opts.add_requirement("material", material.c_str());

	MeshRendererDesc mrd;
	mrd.mesh_resource     = sjson::parse_resource_name(obj["mesh_resource"]);
	mrd.material_resource = sjson::parse_resource_name(obj["material"]);
	mrd.geometry_name     = sjson::parse_string_id    (obj["geometry_name"]);
	mrd.visible           = sjson::parse_bool         (obj["visible"]);
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

static s32 compile_sprite_renderer(Buffer& output, const char* json, CompileOptions& opts)
{
	TempAllocator4096 ta;
	JsonObject obj(ta);
	sjson::parse(obj, json);

	DynamicString sprite_resource(ta);
	sjson::parse_string(sprite_resource, obj["sprite_resource"]);
	DATA_COMPILER_ASSERT_RESOURCE_EXISTS("sprite"
		, sprite_resource.c_str()
		, opts
		);
	opts.add_requirement("sprite", sprite_resource.c_str());

	DynamicString material(ta);
	sjson::parse_string(material, obj["material"]);
	DATA_COMPILER_ASSERT_RESOURCE_EXISTS("material"
		, material.c_str()
		, opts
		);
	opts.add_requirement("material", material.c_str());

	SpriteRendererDesc srd;
	srd.sprite_resource   = sjson::parse_resource_name(obj["sprite_resource"]);
	srd.material_resource = sjson::parse_resource_name(obj["material"]);
	srd.layer             = sjson::parse_int          (obj["layer"]);
	srd.depth             = sjson::parse_int          (obj["depth"]);
	srd.visible           = sjson::parse_bool         (obj["visible"]);
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

static s32 compile_light(Buffer& output, const char* json, CompileOptions& opts)
{
	TempAllocator4096 ta;
	JsonObject obj(ta);
	sjson::parse(obj, json);

	DynamicString type(ta);
	sjson::parse_string(type, obj["type"]);

	LightType::Enum lt = light_name_to_enum(type.c_str());
	DATA_COMPILER_ASSERT(lt != LightType::COUNT
		, opts
		, "Unknown light type: '%s'"
		, type.c_str()
		);

	LightDesc ld;
	ld.type       = lt;
	ld.range      = sjson::parse_float  (obj["range"]);
	ld.intensity  = sjson::parse_float  (obj["intensity"]);
	ld.spot_angle = sjson::parse_float  (obj["spot_angle"]);
	ld.color      = sjson::parse_vector3(obj["color"]);

	FileBuffer fb(output);
	BinaryWriter bw(fb);
	bw.write(ld.type);
	bw.write(ld.range);
	bw.write(ld.intensity);
	bw.write(ld.spot_angle);
	bw.write(ld.color);
	return 0;
}

static s32 compile_script(Buffer& output, const char* json, CompileOptions& opts)
{
	TempAllocator4096 ta;
	JsonObject obj(ta);
	sjson::parse(obj, json);

	DynamicString script_resource(ta);
	sjson::parse_string(script_resource, obj["script_resource"]);
	DATA_COMPILER_ASSERT_RESOURCE_EXISTS("lua"
		, script_resource.c_str()
		, opts
		);
	opts.add_requirement("lua", script_resource.c_str());

	ScriptDesc sd;
	sd.script_resource = sjson::parse_resource_name(obj["script_resource"]);

	FileBuffer fb(output);
	BinaryWriter bw(fb);
	bw.write(sd.script_resource);
	return 0;
}

static s32 compile_animation_state_machine(Buffer& output, const char* json, CompileOptions& opts)
{
	TempAllocator4096 ta;
	JsonObject obj(ta);
	sjson::parse(obj, json);

	DynamicString state_machine_resource(ta);
	sjson::parse_string(state_machine_resource, obj["state_machine_resource"]);
	DATA_COMPILER_ASSERT_RESOURCE_EXISTS("state_machine"
		, state_machine_resource.c_str()
		, opts
		);
	opts.add_requirement("state_machine", state_machine_resource.c_str());

	AnimationStateMachineDesc asmd;
	asmd.state_machine_resource = sjson::parse_resource_name(obj["state_machine_resource"]);

	FileBuffer fb(output);
	BinaryWriter bw(fb);
	bw.write(asmd.state_machine_resource);
	return 0;
}

UnitCompiler::UnitCompiler(CompileOptions& opts)
	: _opts(opts)
	, _num_units(0)
	, _component_data(default_allocator())
	, _component_info(default_allocator())
	, _unit_names(default_allocator())
	, _unit_parents(default_allocator())
{
	register_component_compiler("transform",               &compile_transform,                           0.0f);
	register_component_compiler("camera",                  &compile_camera,                              1.0f);
	register_component_compiler("mesh_renderer",           &compile_mesh_renderer,                       1.0f);
	register_component_compiler("sprite_renderer",         &compile_sprite_renderer,                     1.0f);
	register_component_compiler("light",                   &compile_light,                               1.0f);
	register_component_compiler("script",                  &compile_script,                              1.0f);
	register_component_compiler("collider",                &physics_resource_internal::compile_collider, 1.0f);
	register_component_compiler("actor",                   &physics_resource_internal::compile_actor,    2.0f);
	register_component_compiler("joint",                   &physics_resource_internal::compile_joint,    3.0f);
	register_component_compiler("animation_state_machine", &compile_animation_state_machine,             1.0f);
}

UnitCompiler::~UnitCompiler()
{
}

Buffer UnitCompiler::read_unit(const char* path)
{
	Buffer buf = _opts.read(path);
	array::push_back(buf, '\0');
	return buf;
}

s32 UnitCompiler::compile_unit(const char* path)
{
	return compile_unit_from_json(array::begin(read_unit(path)), UINT32_MAX);
}

u32 object_index_from_id(const JsonArray& objects, const Guid& id)
{
	for (u32 i = 0; i < array::size(objects); ++i)
	{
		TempAllocator512 ta;
		JsonObject obj(ta);
		sjson::parse(obj, objects[i]);
		if (sjson::parse_guid(obj["id"]) == id)
			return i;
	}

	return UINT32_MAX;
}

s32 UnitCompiler::collect_units(Buffer& data, Array<u32>& prefabs, const char* json)
{
	u32 prefab_offt = array::size(data);
	array::push(data, json, strlen32(json));
	array::push_back(data, '\0');
	array::push_back(prefabs, prefab_offt);

	TempAllocator4096 ta;
	JsonObject prefab(ta);
	sjson::parse(prefab, json);

	if (json_object::has(prefab, "prefab"))
	{
		TempAllocator512 ta;
		DynamicString path(ta);
		sjson::parse_string(path, prefab["prefab"]);
		DATA_COMPILER_ASSERT_RESOURCE_EXISTS("unit"
			, path.c_str()
			, _opts
			);
		path += ".unit";

		Buffer buf = read_unit(path.c_str());
		return 1 + collect_units(data, prefabs, array::begin(buf));
	}

	return 1;
}

s32 UnitCompiler::compile_unit_from_json(const char* json, const u32 parent)
{
	Buffer data(default_allocator());
	Array<u32> offsets(default_allocator()); // Offsets to JSON objects into data

	// offsets[  0] = { prefab = ..., comps = .., mods = ... } <- Original unit
	// offsets[  1] = { prefab = ..., ... }                    <- Prefab of the original unit
	// offsets[  2] = { prefab = ..., ... }                    <- Prefab of the prefab of the original unit
	// offsets[n-1] = { prefab = nil, ... }                    <- Root unit
	s32 err = 0;
	err = collect_units(data, offsets, json);
	DATA_COMPILER_ENSURE(err >= 0, _opts);

	TempAllocator4096 ta;
	JsonArray merged_components(ta);
	JsonArray merged_components_data(ta);
	JsonArray merged_children(ta);
	u32 num_prefabs = array::size(offsets);

	// Merge components of all prefabs from the root unit up to the unit that
	// started the compilation.
	for (u32 ii = 0; ii < num_prefabs; ++ii)
	{
		JsonObject prefab(ta);
		sjson::parse(prefab, array::begin(data) + offsets[num_prefabs-1 - ii]);

		if (json_object::has(prefab, "components"))
		{
			JsonArray components(ta);
			sjson::parse_array(components, prefab["components"]);

			// Add components
			for (u32 cc = 0; cc < array::size(components); ++cc)
			{
				JsonObject component(ta);
				sjson::parse_object(component, components[cc]);

				array::push_back(merged_components, components[cc]);
				array::push_back(merged_components_data, component["data"]);
			}
		}

		if (json_object::has(prefab, "deleted_components"))
		{
			JsonObject deleted_components(ta);
			sjson::parse_object(deleted_components, prefab["deleted_components"]);

			// Delete components
			auto cur = json_object::begin(deleted_components);
			auto end = json_object::end(deleted_components);
			for (; cur != end; ++cur)
			{
				JSON_OBJECT_SKIP_HOLE(deleted_components, cur);

				auto key = cur->first;

				// Extract GUID from key #xxxxxxxx-xxxx-xxxx-xxxx-xxxxxxxxxxxx
				char guid[37];
				strncpy(guid, key.data() + 1, sizeof(guid) - 1);
				guid[36] = '\0';
				Guid component_id = guid::parse(guid);

				u32 comp_idx = object_index_from_id(merged_components, component_id);
				if (comp_idx != UINT32_MAX)
				{
					u32 comp_last = array::size(merged_components) - 1;
					merged_components[comp_idx] = merged_components[comp_last];
					array::pop_back(merged_components);
					merged_components_data[comp_idx] = merged_components_data[comp_last];
					array::pop_back(merged_components_data);
				}
				else
				{
					char buf[GUID_BUF_LEN];
					DATA_COMPILER_ASSERT(false
						, _opts
						, "Deletion of unexisting component ID: %s\n"
						, guid::to_string(buf, sizeof(buf), component_id)
						);
				}
			}
		}

		if (json_object::has(prefab, "modified_components"))
		{
			JsonObject modified_components(ta);
			sjson::parse(modified_components, prefab["modified_components"]);

			// Modify components
			auto cur = json_object::begin(modified_components);
			auto end = json_object::end(modified_components);
			for (; cur != end; ++cur)
			{
				JSON_OBJECT_SKIP_HOLE(modified_components, cur);

				auto key = cur->first;
				auto val = cur->second;

				// Extract GUID from key #xxxxxxxx-xxxx-xxxx-xxxx-xxxxxxxxxxxx
				char guid[37];
				strncpy(guid, key.data() + 1, sizeof(guid) - 1);
				guid[36] = '\0';
				Guid component_id = guid::parse(guid);

				// Patch component "data" key
				u32 comp_idx = object_index_from_id(merged_components, component_id);
				if (comp_idx != UINT32_MAX)
				{
					JsonObject modified_component(ta);
					sjson::parse_object(modified_component, val);

					merged_components_data[comp_idx] = modified_component["data"];
				}
				else
				{
					char buf[GUID_BUF_LEN];
					DATA_COMPILER_ASSERT(false
						, _opts
						, "Modification of unexisting component ID: %s\n"
						, guid::to_string(buf, sizeof(buf), component_id)
						);
				}
			}
		}

		if (json_object::has(prefab, "children"))
		{
			JsonArray children(ta);
			sjson::parse_array(children, prefab["children"]);
			for (u32 cc = 0; cc < array::size(children); ++cc)
			{
				array::push_back(merged_children, children[cc]);
			}
		}

		if (json_object::has(prefab, "deleted_children"))
		{
			JsonArray deleted_children(ta);
			sjson::parse_array(deleted_children, prefab["deleted_children"]);

			// Delete components
			for (u32 ii = 0; ii < array::size(deleted_children); ++ii)
			{
				JsonObject obj(ta);
				sjson::parse_object(obj, deleted_children[ii]);
				Guid id = sjson::parse_guid(obj["id"]);

				u32 child_index = object_index_from_id(merged_children, id);
				if (child_index != UINT32_MAX)
				{
					u32 child_last = array::size(merged_children) - 1;
					merged_children[child_index] = merged_children[child_last];
					array::pop_back(merged_children);
				}
				else
				{
					char buf[GUID_BUF_LEN];
					DATA_COMPILER_ASSERT(false
						, _opts
						, "Deletion of unexisting child ID: %s\n"
						, guid::to_string(buf, sizeof(buf), id)
						);
				}
			}
		}

		if (ii == 0)
		{
			// Unnamed object hash == 0
			StringId32 name_hash;

			// Parse Level Editor data
			if (json_object::has(prefab, "editor"))
			{
				JsonObject editor(ta);
				sjson::parse(editor, prefab["editor"]);

				if (json_object::has(editor, "name"))
					name_hash = sjson::parse_string_id(editor["name"]);
			}

			array::push_back(_unit_names, name_hash);
		}
	}

	// Compile component data for each component type found in the chain of units.
	for (u32 cc = 0; cc < array::size(merged_components); ++cc)
	{
		const char* val = merged_components[cc];

		TempAllocator512 ta;
		JsonObject component(ta);
		sjson::parse(component, val);

		const StringId32 type = sjson::parse_string_id(component["type"]);

		Buffer component_data(default_allocator());
		err = compile_component(component_data, type, merged_components_data[cc]);
		DATA_COMPILER_ENSURE(err == 0, _opts);

		// Append data to the component data for the given type.
		ComponentTypeData component_types_deffault(default_allocator());
		ComponentTypeData& ctd = const_cast<ComponentTypeData&>(hash_map::get(_component_data, type, component_types_deffault));

		// One component per unit max.
		auto cur = array::begin(ctd._unit_index);
		auto end = array::end(ctd._unit_index);
		if (std::find(cur, end, _num_units) != end)
		{
			char buf[STRING_ID32_BUF_LEN];
			DATA_COMPILER_ASSERT(false
				, _opts
				, "Unit already has a component of type: %s"
				, type.to_string(buf, sizeof(buf))
				);
		}

		array::push(ctd._data, array::begin(component_data), array::size(component_data));
		array::push_back(ctd._unit_index, _num_units);
		++ctd._num;
	}
	array::push_back(_unit_parents, parent);
	++_num_units;

	err = compile_units_array(merged_children, _num_units-1);
	DATA_COMPILER_ENSURE(err == 0, _opts);
	return 0;
}

s32 UnitCompiler::compile_units_array(const JsonArray& units, const u32 parent)
{
	for (u32 i = 0; i < array::size(units); ++i)
	{
		s32 err = compile_unit_from_json(units[i], parent);
		DATA_COMPILER_ENSURE(err == 0, _opts);
	}
	return 0;
}

s32 UnitCompiler::compile_units_array(const char* json, const u32 parent)
{
	TempAllocator4096 ta;
	JsonArray units(ta);
	sjson::parse_array(units, json);
	return compile_units_array(units, parent);
}

Buffer UnitCompiler::blob()
{
	Buffer output(default_allocator());
	FileBuffer fb(output);
	BinaryWriter bw(fb);

	// Count component types
	u32 num_component_types = 0;
	auto cur = hash_map::begin(_component_data);
	auto end = hash_map::end(_component_data);
	for (; cur != end; ++cur)
	{
		HASH_MAP_SKIP_HOLE(_component_data, cur);

		if (cur->second._num > 0)
			++num_component_types;
	}

	// Write header
	UnitResource ur;
	ur.version = RESOURCE_HEADER(RESOURCE_VERSION_UNIT);
	ur.num_units = _num_units;
	ur.num_component_types = num_component_types;

	bw.write(ur.version);
	bw.write(ur.num_units);
	bw.write(ur.num_component_types);

	// Write parents
	for (u32 ii = 0; ii < _num_units; ++ii)
		bw.write(_unit_parents[ii]);

	for (u32 ii = 0; ii < array::size(_component_info); ++ii)
	{
		const StringId32 type        = _component_info[ii]._type;
		const ComponentTypeData& ctd = hash_map::get(_component_data, type, ComponentTypeData(default_allocator()));

		const Buffer& data           = ctd._data;
		const Array<u32>& unit_index = ctd._unit_index;
		const u32 num                = ctd._num;

		if (num == 0)
			continue;

		// Write component data
		ComponentData cd;
		cd.type = type;
		cd.num_instances = num;
		cd.data_size = array::size(data);

		bw.align(alignof(cd));
		bw.write(cd.type);
		bw.write(cd.num_instances);
		bw.write(cd.data_size);
		for (u32 jj = 0; jj < array::size(unit_index); ++jj)
			bw.write(unit_index[jj]);
		bw.align(16);
		bw.write(array::begin(data), array::size(data));
	}

	return output;
}

void UnitCompiler::register_component_compiler(const char* type, CompileFunction fn, f32 spawn_order)
{
	register_component_compiler(StringId32(type), fn, spawn_order);
}

void UnitCompiler::register_component_compiler(StringId32 type, CompileFunction fn, f32 spawn_order)
{
	ComponentTypeData ctd(default_allocator());
	ctd._compiler = fn;

	ComponentTypeInfo cti;
	cti._type = type;
	cti._spawn_order = spawn_order;

	hash_map::set(_component_data, type, ctd);

	array::push_back(_component_info, cti);
	std::sort(array::begin(_component_info), array::end(_component_info));
}

s32 UnitCompiler::compile_component(Buffer& output, StringId32 type, const char* json)
{
	DATA_COMPILER_ASSERT(hash_map::has(_component_data, type), _opts, "Unknown component");

	return hash_map::get(_component_data, type, ComponentTypeData(default_allocator()))._compiler(output, json, _opts);
}

} // namespace crown

#endif // CROWN_CAN_COMPILE
