/*
 * Copyright (c) 2012-2016 Daniele Bartolini and individual contributors.
 * License: https://github.com/taylor001/crown/blob/master/LICENSE
 */

#include "array.h"
#include "compile_options.h"
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
	for (uint32_t i = 0; i < CE_COUNTOF(s_projection); ++i)
	{
		if (strcmp(name, s_projection[i].name) == 0)
			return s_projection[i].type;
	}

	return ProjectionType::COUNT;
}

static LightType::Enum light_name_to_enum(const char* name)
{
	for (uint32_t i = 0; i < CE_COUNTOF(s_light); ++i)
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

	MeshRendererDesc mrd;
	mrd.mesh_resource     = sjson::parse_resource_id(obj["mesh_resource"]);
	mrd.mesh_name         = sjson::parse_string_id  (obj["mesh_name"]);
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
	sjson::parse_string(obj["light"], type);

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

	// FIXME: remove conversion to radians
	ld.spot_angle = to_rad(ld.spot_angle);

	Buffer buf(default_allocator());
	array::push(buf, (char*)&ld, sizeof(ld));
	return buf;
}

UnitCompiler::UnitCompiler(CompileOptions& opts)
	: _opts(opts)
	, _num_units(0)
	, _component_data(default_allocator())
{
	register_component_compiler("transform",       &compile_transform, 0.0f);
	register_component_compiler("camera",          &compile_camera, 1.0f);
	register_component_compiler("mesh_renderer",   &compile_mesh_renderer, 1.0f);
	register_component_compiler("sprite_renderer", &compile_sprite_renderer, 1.0f);
	register_component_compiler("light",           &compile_light, 1.0f);
	register_component_compiler("controller",      &physics_resource::compile_controller, 1.0f);
	register_component_compiler("collider",        &physics_resource::compile_collider, 1.0f);
	register_component_compiler("actor",           &physics_resource::compile_actor, 1.0f);
	register_component_compiler("joint",           &physics_resource::compile_joint, 1.0f);
}

void UnitCompiler::compile_unit(const char* path)
{
	Buffer buf = _opts.read(path);
	array::push_back(buf, '\0');
	array::pop_back(buf);
	compile_unit_from_json(array::begin(buf));
}

void UnitCompiler::compile_unit_from_json(const char* json)
{
	TempAllocator4096 ta;
	JsonObject obj(ta);
	sjson::parse(json, obj);

	const char* prefab = map::get(obj, FixedString("prefab"), (const char*)NULL);
	if (prefab)
	{
		TempAllocator512 ta;
		DynamicString path(ta);
		sjson::parse_string(prefab, path);

		path += "." UNIT_EXTENSION;
		compile_unit(path.c_str());
	}

	const char* components = map::get(obj, FixedString("components"), (const char*)NULL);
	if (components)
	{
		JsonObject keys(ta);
		sjson::parse(components, keys);

		auto begin = map::begin(keys);
		auto end = map::end(keys);

		for (; begin != end; ++begin)
		{
			const char* value = begin->pair.second;

			TempAllocator512 ta;
			JsonObject component(ta);
			sjson::parse(value, component);

			const StringId32 type = sjson::parse_string_id(component["type"]);

			Buffer buf = compile_component(type, value);
			add_component_data(type, buf, _num_units);
		}

		++_num_units;
	}
}

void UnitCompiler::compile_multiple_units(const char* json)
{
	TempAllocator4096 ta;
	JsonObject obj(ta);
	sjson::parse(json, obj);

	const JsonObject::Node* begin = map::begin(obj);
	const JsonObject::Node* end = map::end(obj);

	for (; begin != end; ++begin)
	{
		const char* unit = begin->pair.second;

		compile_unit_from_json(unit);
	}
}

Buffer UnitCompiler::get()
{
	UnitResource ur;
	ur.version = UNIT_VERSION;
	ur.num_units = _num_units;
	ur.num_component_types = sort_map::size(_component_data);

	Buffer buf(default_allocator());
	array::push(buf, (char*)&ur, sizeof(ur));

	const SortMap<StringId32, ComponentTypeData>::Entry* begin = sort_map::begin(_component_data);
	const SortMap<StringId32, ComponentTypeData>::Entry* end = sort_map::end(_component_data);

	while (begin != end)
	{
		const StringId32 type             = (end-1)->pair.first;
		const Buffer& data                = (end-1)->pair.second._data;
		const Array<uint32_t>& unit_index = (end-1)->pair.second._unit_index;
		const uint32_t num                = (end-1)->pair.second._num;

		ComponentData cd;
		cd.type = type._id;
		cd.num_instances = num;
		cd.size = array::size(data) + sizeof(uint32_t)*array::size(unit_index);

		array::push(buf, (char*)&cd, sizeof(cd));
		array::push(buf, (char*)array::begin(unit_index), sizeof(uint32_t)*array::size(unit_index));
		array::push(buf, array::begin(data), array::size(data));

		--end;
	}

	return buf;
}

void UnitCompiler::add_component_data(StringId32 type, const Buffer& data, uint32_t unit_index)
{
	ComponentTypeData& ctd = const_cast<ComponentTypeData&>(sort_map::get(_component_data, type, ComponentTypeData(default_allocator())));

	array::push(ctd._data, array::begin(data), array::size(data));
	array::push_back(ctd._unit_index, unit_index);
	++ctd._num;
}

void UnitCompiler::register_component_compiler(const char* type, CompileFunction fn, float spawn_order)
{
	ComponentTypeData ctd(default_allocator());
	ctd._compiler = fn;

	sort_map::set(_component_data, StringId32(type), ctd);
	sort_map::sort(_component_data);
}

Buffer UnitCompiler::compile_component(StringId32 type, const char* json)
{
	RESOURCE_COMPILER_ASSERT(sort_map::has(_component_data, type), _opts, "Unknown component");

	return sort_map::get(_component_data, type, ComponentTypeData(default_allocator()))._compiler(json, _opts);
}

} // namespace crown
