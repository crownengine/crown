/*
 * Copyright (c) 2012-2016 Daniele Bartolini and individual contributors.
 * License: https://github.com/taylor001/crown/blob/master/LICENSE
 */

#include "array.h"
#include "compile_options.h"
#include "macros.h"
#include "map.h"
#include "math_utils.h"
#include "physics_resource.h"
#include "sjson.h"
#include "sort_map.h"
#include "temp_allocator.h"
#include "unit_compiler.h"
#include "unit_resource.h"
#include "world_types.h"

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
CE_STATIC_ASSERT(CE_COUNTOF(s_projection) == ProjectionType::COUNT);

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
CE_STATIC_ASSERT(CE_COUNTOF(s_light) == LightType::COUNT);

static ProjectionType::Enum projection_name_to_enum(const char* name)
{
	for (u32 i = 0; i < CE_COUNTOF(s_projection); ++i)
	{
		if (strcmp(name, s_projection[i].name) == 0)
			return s_projection[i].type;
	}

	return ProjectionType::COUNT;
}

static LightType::Enum light_name_to_enum(const char* name)
{
	for (u32 i = 0; i < CE_COUNTOF(s_light); ++i)
	{
		if (strcmp(name, s_light[i].name) == 0)
			return s_light[i].type;
	}

	return LightType::COUNT;
}

static Buffer compile_transform(const char* json, CompileOptions& opts)
{
	TempAllocator4096 ta;
	JsonObject obj(ta);
	sjson::parse(json, obj);

	TransformDesc td;
	td.position = sjson::parse_vector3   (obj["position"]);
	td.rotation = sjson::parse_quaternion(obj["rotation"]);
	td.scale    = sjson::parse_vector3   (obj["scale"]);

	Buffer buf(default_allocator());
	array::push(buf, (char*)&td, sizeof(td));
	return buf;
}

static Buffer compile_camera(const char* json, CompileOptions& opts)
{
	TempAllocator4096 ta;
	JsonObject obj(ta);
	sjson::parse(json, obj);

	DynamicString type(ta);
	sjson::parse_string(obj["projection"], type);

	ProjectionType::Enum pt = projection_name_to_enum(type.c_str());
	RESOURCE_COMPILER_ASSERT(pt != ProjectionType::COUNT
		, opts
		, "Unknown projection type: '%s'"
		, type.c_str()
		);

	CameraDesc cd;
	cd.type       = pt;
	cd.fov        = sjson::parse_float(obj["fov"]);
	cd.near_range = sjson::parse_float(obj["near_range"]);
	cd.far_range  = sjson::parse_float(obj["far_range"]);

	Buffer buf(default_allocator());
	array::push(buf, (char*)&cd, sizeof(cd));
	return buf;
}

static Buffer compile_mesh_renderer(const char* json, CompileOptions& opts)
{
	TempAllocator4096 ta;
	JsonObject obj(ta);
	sjson::parse(json, obj);

	DynamicString mesh_resource(ta);
	sjson::parse_string(obj["mesh_resource"], mesh_resource);
	RESOURCE_COMPILER_ASSERT_RESOURCE_EXISTS(RESOURCE_EXTENSION_MESH, mesh_resource.c_str(), opts);

	MeshRendererDesc mrd;
	mrd.mesh_resource     = sjson::parse_resource_id(obj["mesh_resource"]);
	mrd.geometry_name     = sjson::parse_string_id  (obj["geometry_name"]);
	mrd.material_resource = sjson::parse_resource_id(obj["material"]);
	mrd.visible           = sjson::parse_bool       (obj["visible"]);

	Buffer buf(default_allocator());
	array::push(buf, (char*)&mrd, sizeof(mrd));
	return buf;
}

static Buffer compile_sprite_renderer(const char* json, CompileOptions& opts)
{
	TempAllocator4096 ta;
	JsonObject obj(ta);
	sjson::parse(json, obj);

	DynamicString resource(ta);
	sjson::parse_string(obj["resource"], resource);
	RESOURCE_COMPILER_ASSERT_RESOURCE_EXISTS(RESOURCE_EXTENSION_SPRITE, resource.c_str(), opts);

	SpriteRendererDesc srd;
	srd.sprite_resource   = sjson::parse_resource_id(obj["resource"]);
	srd.material_resource = sjson::parse_resource_id(obj["material"]);
	srd.visible           = sjson::parse_bool       (obj["visible"]);

	Buffer buf(default_allocator());
	array::push(buf, (char*)&srd, sizeof(srd));
	return buf;
}

static Buffer compile_light(const char* json, CompileOptions& opts)
{
	TempAllocator4096 ta;
	JsonObject obj(ta);
	sjson::parse(json, obj);

	DynamicString type(ta);
	sjson::parse_string(obj["type"], type);

	LightType::Enum lt = light_name_to_enum(type.c_str());
	RESOURCE_COMPILER_ASSERT(lt != LightType::COUNT
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

	Buffer buf(default_allocator());
	array::push(buf, (char*)&ld, sizeof(ld));
	return buf;
}

UnitCompiler::UnitCompiler(CompileOptions& opts)
	: _opts(opts)
	, _num_units(0)
	, _component_data(default_allocator())
	, _component_info(default_allocator())
{
	register_component_compiler("transform",       &compile_transform,                    0.0f);
	register_component_compiler("camera",          &compile_camera,                       1.0f);
	register_component_compiler("mesh_renderer",   &compile_mesh_renderer,                1.0f);
	register_component_compiler("sprite_renderer", &compile_sprite_renderer,              1.0f);
	register_component_compiler("light",           &compile_light,                        1.0f);
	register_component_compiler("controller",      &physics_resource::compile_controller, 1.0f);
	register_component_compiler("collider",        &physics_resource::compile_collider,   1.0f);
	register_component_compiler("actor",           &physics_resource::compile_actor,      2.0f);
	register_component_compiler("joint",           &physics_resource::compile_joint,      3.0f);
}

Buffer UnitCompiler::read_unit(const char* path)
{
	Buffer buf = _opts.read(path);
	array::push_back(buf, '\0');
	return buf;
}

void UnitCompiler::compile_unit(const char* path)
{
	compile_unit_from_json(array::begin(read_unit(path)));
}

void UnitCompiler::compile_unit_from_json(const char* json)
{
	Buffer data(default_allocator());
	array::reserve(data, 1024*1024);

	u32 num_prefabs = 1;

	TempAllocator4096 ta;
	JsonObject prefabs[4] = { JsonObject(ta), JsonObject(ta), JsonObject(ta), JsonObject(ta) };
	sjson::parse(json, prefabs[0]);

	for (u32 i = 0; i < CE_COUNTOF(prefabs); ++i, ++num_prefabs)
	{
		const JsonObject& prefab = prefabs[i];

		if (!map::has(prefab, FixedString("prefab")))
			break;

		TempAllocator512 ta;
		DynamicString path(ta);
		sjson::parse_string(prefab["prefab"], path);
		RESOURCE_COMPILER_ASSERT_RESOURCE_EXISTS(RESOURCE_EXTENSION_UNIT, path.c_str(), _opts);
		path += "." RESOURCE_EXTENSION_UNIT;

		Buffer buf = read_unit(path.c_str());
		const char* d = array::end(data);
		array::push(data, array::begin(buf), array::size(buf));
		sjson::parse(d, prefabs[i + 1]);
	}


	JsonObject& prefab_root = prefabs[num_prefabs - 1];
	JsonObject prefab_root_components(ta);
	sjson::parse(prefab_root["components"], prefab_root_components);

	if (num_prefabs > 1)
	{
		// Merge prefabs' components
		for (u32 i = 0; i < num_prefabs; ++i)
		{
			const JsonObject& prefab = prefabs[num_prefabs - i - 1];

			if (!map::has(prefab, FixedString("modified_components")))
				continue;

			JsonObject modified_components(ta);
			sjson::parse(prefab["modified_components"], modified_components);

			auto begin = map::begin(modified_components);
			auto end = map::end(modified_components);
			for (; begin != end; ++begin)
			{
				const FixedString key = begin->pair.first;
				const FixedString id(&key.data()[1], key.length()-1);
				const char* value = begin->pair.second;

				// FIXME
				map::remove(prefab_root_components, id);
				map::set(prefab_root_components, id, value);
			}
		}
	}

	if (map::size(prefab_root_components) > 0)
	{
		auto begin = map::begin(prefab_root_components);
		auto end = map::end(prefab_root_components);
		for (; begin != end; ++begin)
		{
			const char* value = begin->pair.second;

			TempAllocator512 ta;
			JsonObject component(ta);
			sjson::parse(value, component);

			const StringId32 type = sjson::parse_string_id(component["type"]);

			Buffer buf = compile_component(type, component["data"]);
			add_component_data(type, buf, _num_units);
		}
	}

	++_num_units;
}

void UnitCompiler::compile_multiple_units(const char* json)
{
	TempAllocator4096 ta;
	JsonObject obj(ta);
	sjson::parse(json, obj);

	auto begin = map::begin(obj);
	auto end = map::end(obj);

	for (; begin != end; ++begin)
		compile_unit_from_json(begin->pair.second);
}

Buffer UnitCompiler::blob()
{
	UnitResource ur;
	ur.version = RESOURCE_VERSION_UNIT;
	ur.num_units = _num_units;
	ur.num_component_types = 0;

	auto begin = sort_map::begin(_component_data);
	auto end = sort_map::end(_component_data);

	for (; begin != end; ++begin)
	{
		const u32 num = begin->pair.second._num;

		if (num > 0)
			++ur.num_component_types;
	}

	Buffer buf(default_allocator());
	array::push(buf, (char*)&ur, sizeof(ur));

	for (u32 i = 0; i < array::size(_component_info); ++i)
	{
		const StringId32 type        = _component_info[i]._type;
		const ComponentTypeData& ctd = sort_map::get(_component_data, type, ComponentTypeData(default_allocator()));

		const Buffer& data           = ctd._data;
		const Array<u32>& unit_index = ctd._unit_index;
		const u32 num                = ctd._num;

		if (num > 0)
		{
			ComponentData cd;
			cd.type = type;
			cd.num_instances = num;
			cd.size = array::size(data) + sizeof(u32)*array::size(unit_index);

			array::push(buf, (char*)&cd, sizeof(cd));
			array::push(buf, (char*)array::begin(unit_index), sizeof(u32)*array::size(unit_index));
			array::push(buf, array::begin(data), array::size(data));
		}
	}

	return buf;
}

void UnitCompiler::add_component_data(StringId32 type, const Buffer& data, u32 unit_index)
{
	ComponentTypeData& ctd = const_cast<ComponentTypeData&>(sort_map::get(_component_data, type, ComponentTypeData(default_allocator())));

	array::push(ctd._data, array::begin(data), array::size(data));
	array::push_back(ctd._unit_index, unit_index);
	++ctd._num;
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

	sort_map::set(_component_data, type, ctd);
	sort_map::sort(_component_data);

	array::push_back(_component_info, cti);
	std::sort(array::begin(_component_info), array::end(_component_info));
}

Buffer UnitCompiler::compile_component(StringId32 type, const char* json)
{
	RESOURCE_COMPILER_ASSERT(sort_map::has(_component_data, type), _opts, "Unknown component");

	return sort_map::get(_component_data, type, ComponentTypeData(default_allocator()))._compiler(json, _opts);
}

} // namespace crown
