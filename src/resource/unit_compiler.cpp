/*
 * Copyright (c) 2012-2026 Daniele Bartolini et al.
 * SPDX-License-Identifier: MIT
 */

#include "config.h"

#if CROWN_CAN_COMPILE
#include "core/containers/array.inl"
#include "core/containers/hash_map.inl"
#include "core/containers/vector.inl"
#include "core/filesystem/file_buffer.inl"
#include "core/guid.inl"
#include "core/json/json.h"
#include "core/json/json_object.inl"
#include "core/json/sjson.h"
#include "core/math/math.h"
#include "core/memory/temp_allocator.inl"
#include "core/strings/dynamic_string.inl"
#include "core/strings/string_id.inl"
#include "device/log.h"
#include "resource/compile_options.inl"
#include "resource/physics_resource.h"
#include "resource/resource_id.inl"
#include "resource/unit_compiler.h"
#include "resource/unit_resource.h"
#include "world/types.h"
#include <algorithm>

LOG_SYSTEM(UNIT_COMPILER, "unit_compiler")

namespace crown
{
bool operator==(const ComponentKey &a, const ComponentKey &b)
{
	return a.component_id == b.component_id
		&& a.root_unit_index == b.root_unit_index
		;
}

u32 hash<ComponentKey>::operator()(const ComponentKey &key) const
{
	return u32(key.component_id.data1 ^ key.component_id.data2 ^ key.root_unit_index);
}

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

struct LodFadeModeInfo
{
	const char *name;
	LodFadeMode::Enum type;
};

static const LodFadeModeInfo s_lod_fade_mode[] =
{
	{ "none",      LodFadeMode::NONE      },
	{ "crossfade", LodFadeMode::CROSSFADE }
};
CE_STATIC_ASSERT(countof(s_lod_fade_mode) == LodFadeMode::COUNT);

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

static LodFadeMode::Enum lod_fade_mode_name_to_enum(const char *name)
{
	for (u32 i = 0; i < countof(s_lod_fade_mode); ++i) {
		if (strcmp(name, s_lod_fade_mode[i].name) == 0)
			return s_lod_fade_mode[i].type;
	}

	return LodFadeMode::COUNT;
}

static s32 compile_transform(Buffer &output, UnitCompiler &compiler, FlatJsonObject &obj, CompileOptions &opts)
{
	CE_UNUSED_2(compiler, opts);

	TransformDesc td;
	td.position = RETURN_IF_ERROR(sjson::parse_vector3   (flat_json_object::get(obj, "data.position")));
	td.rotation = RETURN_IF_ERROR(sjson::parse_quaternion(flat_json_object::get(obj, "data.rotation")));
	td.scale    = RETURN_IF_ERROR(sjson::parse_vector3   (flat_json_object::get(obj, "data.scale")));
	if (flat_json_object::has(obj, "data.name")) {
		td.name = RETURN_IF_ERROR(sjson::parse_string_id(flat_json_object::get(obj, "data.name")));
	}

	FileBuffer fb(output);
	BinaryWriter bw(fb);
	bw.write(td.position);
	bw.write(td.rotation);
	bw.write(td.scale);
	bw.write(td.name);
	return 0;
}

static s32 compile_camera(Buffer &output, UnitCompiler &compiler, FlatJsonObject &obj, CompileOptions &opts)
{
	CE_UNUSED(compiler);

	TempAllocator4096 ta;
	DynamicString type(ta);
	RETURN_IF_ERROR(sjson::parse_string(type, flat_json_object::get(obj, "data.projection")));

	ProjectionType::Enum pt = projection_name_to_enum(type.c_str());
	RETURN_IF_FALSE(pt != ProjectionType::COUNT
		, opts
		, "Unknown projection type: '%s'"
		, type.c_str()
		);

	CameraDesc cd;
	cd.type       = pt;
	cd.fov        = RETURN_IF_ERROR(sjson::parse_float(flat_json_object::get(obj, "data.fov")));
	cd.near_range = RETURN_IF_ERROR(sjson::parse_float(flat_json_object::get(obj, "data.near_range")));
	cd.far_range  = RETURN_IF_ERROR(sjson::parse_float(flat_json_object::get(obj, "data.far_range")));

	FileBuffer fb(output);
	BinaryWriter bw(fb);
	bw.write(cd.type);
	bw.write(cd.fov);
	bw.write(cd.near_range);
	bw.write(cd.far_range);
	return 0;
}

static s32 compile_mesh_renderer(Buffer &output, UnitCompiler &compiler, FlatJsonObject &obj, CompileOptions &opts)
{
	CE_UNUSED(compiler);

	TempAllocator4096 ta;
	DynamicString mesh_resource(ta);
	RETURN_IF_ERROR(sjson::parse_string(mesh_resource, flat_json_object::get(obj, "data.mesh_resource")));

	RETURN_IF_RESOURCE_MISSING("mesh", mesh_resource.c_str(), opts);
	opts.add_requirement("mesh", mesh_resource.c_str());

	DynamicString material(ta);
	RETURN_IF_ERROR(sjson::parse_string(material, flat_json_object::get(obj, "data.material")));
	RETURN_IF_RESOURCE_MISSING("material"
		, material.c_str()
		, opts
		);
	opts.add_requirement("material", material.c_str());

	MeshRendererDesc mrd;
	mrd.mesh_resource     = RETURN_IF_ERROR(sjson::parse_resource_name(flat_json_object::get(obj, "data.mesh_resource")));
	mrd.material_resource = RETURN_IF_ERROR(sjson::parse_resource_name(flat_json_object::get(obj, "data.material")));
	mrd.geometry_name     = RETURN_IF_ERROR(sjson::parse_string_id    (flat_json_object::get(obj, "data.geometry_name")));
	mrd.flags = 0u;
	{
		bool visible = RETURN_IF_ERROR(sjson::parse_bool(flat_json_object::get(obj, "data.visible")));
		mrd.flags |= visible ? RenderableFlags::VISIBLE : 0u;
	}
	if (flat_json_object::has(obj, "data.cast_shadows")) {
		bool cast_shadows = RETURN_IF_ERROR(sjson::parse_bool(flat_json_object::get(obj, "data.cast_shadows")));
		mrd.flags |= cast_shadows ? RenderableFlags::SHADOW_CASTER : 0u;
	} else {
		mrd.flags |= RenderableFlags::SHADOW_CASTER;
	}

	FileBuffer fb(output);
	BinaryWriter bw(fb);
	bw.write(mrd.mesh_resource);
	bw.write(mrd.material_resource);
	bw.write(mrd.geometry_name);
	bw.write(mrd.flags);
	return 0;
}

static s32 compile_sprite_renderer(Buffer &output, UnitCompiler &compiler, FlatJsonObject &obj, CompileOptions &opts)
{
	CE_UNUSED(compiler);

	TempAllocator4096 ta;
	DynamicString sprite_resource(ta);
	RETURN_IF_ERROR(sjson::parse_string(sprite_resource, flat_json_object::get(obj, "data.sprite_resource")));
	RETURN_IF_RESOURCE_MISSING("sprite"
		, sprite_resource.c_str()
		, opts
		);
	opts.add_requirement("sprite", sprite_resource.c_str());

	DynamicString material(ta);
	RETURN_IF_ERROR(sjson::parse_string(material, flat_json_object::get(obj, "data.material")));
	RETURN_IF_RESOURCE_MISSING("material"
		, material.c_str()
		, opts
		);
	opts.add_requirement("material", material.c_str());

	SpriteRendererDesc srd;
	srd.sprite_resource   = RETURN_IF_ERROR(sjson::parse_resource_name(flat_json_object::get(obj, "data.sprite_resource")));
	srd.material_resource = RETURN_IF_ERROR(sjson::parse_resource_name(flat_json_object::get(obj, "data.material")));
	srd.layer             = RETURN_IF_ERROR(sjson::parse_int          (flat_json_object::get(obj, "data.layer")));
	srd.depth             = RETURN_IF_ERROR(sjson::parse_int          (flat_json_object::get(obj, "data.depth")));
	srd.flags = 0u;
	{
		bool visible = RETURN_IF_ERROR(sjson::parse_bool(flat_json_object::get(obj, "data.visible")));
		srd.flags = visible ? RenderableFlags::VISIBLE : 0u;
	}
	if (flat_json_object::has(obj, "data.flip_x")) {
		bool flip = RETURN_IF_ERROR(sjson::parse_bool(flat_json_object::get(obj, "data.flip_x")));
		srd.flags |= flip ? SpriteFlags::FLIP_X : 0u;
	}
	if (flat_json_object::has(obj, "data.flip_y")) {
		bool flip = RETURN_IF_ERROR(sjson::parse_bool(flat_json_object::get(obj, "data.flip_y")));
		srd.flags |= flip ? SpriteFlags::FLIP_Y : 0u;
	}

	memset(srd._pad, 0, sizeof(srd._pad));

	FileBuffer fb(output);
	BinaryWriter bw(fb);
	bw.write(srd.sprite_resource);
	bw.write(srd.material_resource);
	bw.write(srd.layer);
	bw.write(srd.depth);
	bw.write(srd.flags);
	bw.write(srd._pad);
	return 0;
}

static s32 compile_lod_group(Buffer &output, UnitCompiler &compiler, FlatJsonObject &obj, CompileOptions &opts)
{
	TempAllocator4096 ta;
	JsonArray lod_levels(ta);
	RETURN_IF_ERROR(sjson::parse_array(lod_levels, flat_json_object::get(obj, "data.lod_levels")));
	RETURN_IF_FALSE(array::size(lod_levels) > 0, opts, "LOD group must have at least one level");

	DynamicString fade_mode_name(ta);
	RETURN_IF_ERROR(sjson::parse_string(fade_mode_name, flat_json_object::get(obj, "data.fade_mode")));
	LodFadeMode::Enum fade_mode = lod_fade_mode_name_to_enum(fade_mode_name.c_str());
	RETURN_IF_FALSE(fade_mode != LodFadeMode::COUNT
		, opts
		, "Unknown LOD group fade mode: '%s'"
		, fade_mode_name.c_str()
		);

	const s32 level = RETURN_IF_ERROR(sjson::parse_int(flat_json_object::get(obj, "data.level")));

	Array<LodDesc> levels(default_allocator());

	for (u32 i = 0; i < array::size(lod_levels); ++i) {
		JsonObject level(ta);
		RETURN_IF_ERROR(sjson::parse_object(level, lod_levels[i]));

		JsonObject level_data(ta);
		RETURN_IF_ERROR(sjson::parse_object(level_data, level["data"]));

		const Guid mesh_renderer_id = RETURN_IF_ERROR(sjson::parse_guid(level_data["mesh_renderer"]));

		LodDesc desc;
		desc.unit_index   = UINT32_MAX;
		desc.screen_size  = RETURN_IF_ERROR(sjson::parse_float(level_data["screen_size"]));

		if (mesh_renderer_id != GUID_ZERO) {
			const u32 root_unit_index = compiler._unit_roots[compiler._current_unit_index];
			const ComponentKey key = { mesh_renderer_id, root_unit_index };
			const StringId32 mesh_renderer_type = hash_map::get(compiler._component_type, key, StringId32());
			RETURN_IF_FALSE(mesh_renderer_type == STRING_ID_32("mesh_renderer", UINT32_C(0xdf017893))
				, opts
				, "LOD level references a non-mesh-renderer component"
				);

			desc.unit_index = hash_map::get(compiler._component_unit_index, key, UINT32_MAX);
			RETURN_IF_FALSE(desc.unit_index != UINT32_MAX
				, opts
				, "LOD level references a mesh renderer outside this unit resource"
				);
		}

		RETURN_IF_FALSE(desc.screen_size >= 0.0f && desc.screen_size <= 1.0f
			, opts
			, "LOD level screen_size height threshold must be in [0, 1]"
			);

		array::push_back(levels, desc);
	}

	std::sort(array::begin(levels)
		, array::end(levels)
		, [](const LodDesc &a, const LodDesc &b) {
			return a.screen_size > b.screen_size;
		}
		);

	LodGroupDesc desc;
	desc.num_levels = array::size(levels);
	desc.fade_mode = fade_mode;
	desc.level = level;

	RETURN_IF_FALSE(desc.level == -1 || (u32)desc.level < desc.num_levels
		, opts
		, "LOD group level must be -1 or a valid LOD index"
		);

	FileBuffer fb(output);
	BinaryWriter bw(fb);
	bw.write(desc.num_levels);
	bw.write(desc.fade_mode);
	bw.write(desc.level);
	for (u32 i = 0; i < array::size(levels); ++i) {
		bw.write(levels[i].unit_index);
		bw.write(levels[i].screen_size);
	}
	return 0;
}

static s32 compile_light(Buffer &output, UnitCompiler &compiler, FlatJsonObject &obj, CompileOptions &opts)
{
	CE_UNUSED(compiler);

	TempAllocator4096 ta;
	DynamicString type(ta);
	RETURN_IF_ERROR(sjson::parse_string(type, flat_json_object::get(obj, "data.type")));

	LightType::Enum lt = light_name_to_enum(type.c_str());
	RETURN_IF_FALSE(lt != LightType::COUNT
		, opts
		, "Unknown light type: '%s'"
		, type.c_str()
		);

	LightDesc ld;
	ld.type        = lt;
	ld.range       = RETURN_IF_ERROR(sjson::parse_float  (flat_json_object::get(obj, "data.range")));
	ld.intensity   = RETURN_IF_ERROR(sjson::parse_float  (flat_json_object::get(obj, "data.intensity")));
	ld.spot_angle  = RETURN_IF_ERROR(sjson::parse_float  (flat_json_object::get(obj, "data.spot_angle")));
	ld.color       = RETURN_IF_ERROR(sjson::parse_vector3(flat_json_object::get(obj, "data.color")));
	ld.shadow_bias = 0.0004f;
	if (flat_json_object::has(obj, "data.shadow_bias")) {
		ld.shadow_bias = RETURN_IF_ERROR(sjson::parse_float(flat_json_object::get(obj, "data.shadow_bias")));
	}
	ld.flags = 0u;
	if (flat_json_object::has(obj, "data.cast_shadows")) {
		bool cast_shadows = RETURN_IF_ERROR(sjson::parse_bool(flat_json_object::get(obj, "data.cast_shadows")));
		ld.flags |= cast_shadows ? RenderableFlags::SHADOW_CASTER : 0u;
	} else {
		ld.flags |= RenderableFlags::SHADOW_CASTER;
	}

	FileBuffer fb(output);
	BinaryWriter bw(fb);
	bw.write(ld.type);
	bw.write(ld.range);
	bw.write(ld.intensity);
	bw.write(ld.spot_angle);
	bw.write(ld.color);
	bw.write(ld.shadow_bias);
	bw.write(ld.flags);
	return 0;
}

static s32 compile_script(Buffer &output, UnitCompiler &compiler, FlatJsonObject &obj, CompileOptions &opts)
{
	CE_UNUSED(compiler);

	TempAllocator4096 ta;
	DynamicString script_resource(ta);
	RETURN_IF_ERROR(sjson::parse_string(script_resource, flat_json_object::get(obj, "data.script_resource")));
	RETURN_IF_RESOURCE_MISSING("lua"
		, script_resource.c_str()
		, opts
		);
	opts.add_requirement("lua", script_resource.c_str());

	ScriptDesc sd;
	sd.script_resource = RETURN_IF_ERROR(sjson::parse_resource_name(flat_json_object::get(obj, "data.script_resource")));
	strcpy(sd.script_resource_name, script_resource.c_str());

	FileBuffer fb(output);
	BinaryWriter bw(fb);
	bw.write(sd.script_resource);
	bw.write(sd.script_resource_name);
	return 0;
}

static s32 compile_animation_state_machine(Buffer &output, UnitCompiler &compiler, FlatJsonObject &obj, CompileOptions &opts)
{
	CE_UNUSED(compiler);

	TempAllocator4096 ta;
	DynamicString state_machine_resource(ta);
	RETURN_IF_ERROR(sjson::parse_string(state_machine_resource, flat_json_object::get(obj, "data.state_machine_resource")));
	RETURN_IF_RESOURCE_MISSING("state_machine"
		, state_machine_resource.c_str()
		, opts
		);
	opts.add_requirement("state_machine", state_machine_resource.c_str());

	AnimationStateMachineDesc asmd;
	asmd.state_machine_resource = RETURN_IF_ERROR(sjson::parse_resource_name(flat_json_object::get(obj, "data.state_machine_resource")));

	FileBuffer fb(output);
	BinaryWriter bw(fb);
	bw.write(asmd.state_machine_resource);
	return 0;
}

static s32 compile_fog(Buffer &output, UnitCompiler &compiler, FlatJsonObject &obj, CompileOptions &opts)
{
	CE_UNUSED_2(compiler, opts);

	TempAllocator4096 ta;

	FogDesc fd;
	fd.color     = RETURN_IF_ERROR(sjson::parse_vector3(flat_json_object::get(obj, "data.color")));
	fd.density   = RETURN_IF_ERROR(sjson::parse_float(flat_json_object::get(obj, "data.density")));
	fd.range_min = RETURN_IF_ERROR(sjson::parse_float(flat_json_object::get(obj, "data.range_min")));
	fd.range_max = RETURN_IF_ERROR(sjson::parse_float(flat_json_object::get(obj, "data.range_max")));
	fd.sun_blend = RETURN_IF_ERROR(sjson::parse_float(flat_json_object::get(obj, "data.sun_blend")));
	fd.enabled   = (f32)RETURN_IF_ERROR(sjson::parse_bool(flat_json_object::get(obj, "data.enabled")));

	FileBuffer fb(output);
	BinaryWriter bw(fb);
	bw.write(fd.color);
	bw.write(fd.density);
	bw.write(fd.range_min);
	bw.write(fd.range_max);
	bw.write(fd.sun_blend);
	bw.write(fd.enabled);
	return 0;
}

static s32 compile_global_lighting(Buffer &output, UnitCompiler &compiler, FlatJsonObject &obj, CompileOptions &opts)
{
	CE_UNUSED(compiler);

	TempAllocator4096 ta;

	DynamicString skydome_map(ta);
	RETURN_IF_ERROR(sjson::parse_string(skydome_map, flat_json_object::get(obj, "data.skydome_map")));
	RETURN_IF_RESOURCE_MISSING("texture"
		, skydome_map.c_str()
		, opts
		);
	opts.add_requirement("texture", skydome_map.c_str());

	GlobalLightingDesc ld;
	ld.skydome_map       = RETURN_IF_ERROR(sjson::parse_resource_name(flat_json_object::get(obj, "data.skydome_map")));
	ld.skydome_intensity = RETURN_IF_ERROR(sjson::parse_float(flat_json_object::get(obj, "data.skydome_intensity")));
	ld.ambient_color     = RETURN_IF_ERROR(sjson::parse_vector3(flat_json_object::get(obj, "data.ambient_color")));

	FileBuffer fb(output);
	BinaryWriter bw(fb);
	bw.write(ld.skydome_map);
	bw.write(ld.skydome_intensity);
	bw.write(ld.ambient_color);
	return 0;
}

static s32 compile_bloom(Buffer &output, UnitCompiler &compiler, FlatJsonObject &obj, CompileOptions &opts)
{
	CE_UNUSED_2(compiler, opts);

	TempAllocator4096 ta;

	BloomDesc desc;
	desc.enabled   = RETURN_IF_ERROR(sjson::parse_bool(flat_json_object::get(obj, "data.enabled")));
	memset(&desc._pad, 0, sizeof(desc._pad));
	desc.threshold = RETURN_IF_ERROR(sjson::parse_float(flat_json_object::get(obj, "data.threshold")));
	desc.weight    = RETURN_IF_ERROR(sjson::parse_float(flat_json_object::get(obj, "data.weight")));
	desc.intensity = RETURN_IF_ERROR(sjson::parse_float(flat_json_object::get(obj, "data.intensity")));

	FileBuffer fb(output);
	BinaryWriter bw(fb);
	bw.write(desc.enabled);
	bw.write(desc._pad);
	bw.write(desc.threshold);
	bw.write(desc.weight);
	bw.write(desc.intensity);
	return 0;
}

static s32 compile_tonemap(Buffer &output, UnitCompiler &compiler, FlatJsonObject &obj, CompileOptions &opts)
{
	CE_UNUSED(compiler);

	TonemapDesc desc;
	StringId32 type = RETURN_IF_ERROR(sjson::parse_string_id(flat_json_object::get(obj, "data.type")));

	if (type == STRING_ID_32("gamma", UINT32_C(0x960fbc97))) {
		desc.type = (f32)TonemapType::GAMMA;
	} else if (type == STRING_ID_32("reinhard", UINT32_C(0x3c25f546))) {
		desc.type = (f32)TonemapType::REINHARD;
	} else if (type == STRING_ID_32("filmic", UINT32_C(0xcebb6592))) {
		desc.type = (f32)TonemapType::FILMIC;
	} else if (type == STRING_ID_32("aces", UINT32_C(0xe426fd33))) {
		desc.type = (f32)TonemapType::ACES;
	} else {
		RETURN_IF_FALSE(false, opts, "Unknown tonemap type");
	}

	memset(&desc.unused, 0, sizeof(desc.unused));

	FileBuffer fb(output);
	BinaryWriter bw(fb);
	bw.write(desc.type);
	bw.write(desc.unused);
	return 0;
}

namespace unit_compiler
{
	Buffer read_unit(const char *path, CompileOptions &opts)
	{
		Buffer buf = opts.read(path);
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
	s32 collect_prefabs(UnitCompiler &c, StringId64 unit_name, const char *unit_json, bool append_data, CompileOptions &opts)
	{
		TempAllocator4096 ta;
		JsonObject prefab(ta);
		RETURN_IF_ERROR(sjson::parse(prefab, unit_json));

		if (json_object::has(prefab, "children")) {
			JsonArray children(ta);
			RETURN_IF_ERROR(sjson::parse_array(children, prefab["children"]));

			for (u32 i = 0; i < array::size(children); ++i) {
				s32 err = collect_prefabs(c, unit_name, children[i], false, opts);
				ENSURE_OR_RETURN(err == 0, opts);
			}
		}

		if (json_object::has(prefab, "prefab")
			&& sjson::type(prefab["prefab"]) == JsonValueType::STRING) {
			TempAllocator512 ta;
			DynamicString path(ta);
			RETURN_IF_ERROR(sjson::parse_string(path, prefab["prefab"]));
			RETURN_IF_RESOURCE_MISSING("unit"
				, path.c_str()
				, opts
				);
			StringId64 name(path.c_str());
			RETURN_IF_FALSE(name != unit_name, opts, "Prefab derives from itself");
			path += ".unit";

			Buffer buf = read_unit(path.c_str(), opts);
			s32 err = collect_prefabs(c, name, array::begin(buf), true, opts);
			ENSURE_OR_RETURN(err == 0, opts);
		}

		if (append_data) {
			u32 prefab_offset = array::size(c._prefab_data);
			array::push(c._prefab_data, unit_json, u32(prefab._end - unit_json));
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
		CE_UNUSED(opts);

		for (u32 i = 0; i < array::size(objects); ++i) {
			TempAllocator512 ta;
			JsonObject obj(ta);
			RETURN_IF_ERROR(sjson::parse(obj, objects[i]));

			if (json_object::has(obj, "id")) {
				Guid id = RETURN_IF_ERROR(sjson::parse_guid(obj["id"]));
				if (id == object_id)
					return i;
			} else {
				Guid id = RETURN_IF_ERROR(sjson::parse_guid(obj["_guid"]));
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

	static u32 find_child_unit_index(Unit *unit, Guid id)
	{
		CE_ENSURE(unit != NULL);

		auto cur = hash_map::begin(unit->_children);
		auto end = hash_map::end(unit->_children);
		for (; cur != end; ++cur) {
			HASH_MAP_SKIP_HOLE(unit->_children, cur);

			if (cur->first == id)
				return cur->second->_index;

			const u32 child_index = find_child_unit_index(cur->second, id);
			if (child_index != UINT32_MAX)
				return child_index;
		}

		return UINT32_MAX;
	}

	static Unit *find_root_unit(UnitCompiler &c, u32 root_unit_index)
	{
		auto cur = hash_map::begin(c._units);
		auto end = hash_map::end(c._units);
		for (; cur != end; ++cur) {
			HASH_MAP_SKIP_HOLE(c._units, cur);

			if (cur->second->_index == root_unit_index)
				return cur->second;
		}

		return NULL;
	}

	static Unit *find_unit_by_index(Unit *unit, u32 unit_index)
	{
		CE_ENSURE(unit != NULL);

		if (unit->_index == unit_index)
			return unit;

		auto cur = hash_map::begin(unit->_children);
		auto end = hash_map::end(unit->_children);
		for (; cur != end; ++cur) {
			HASH_MAP_SKIP_HOLE(unit->_children, cur);

			Unit *child = find_unit_by_index(cur->second, unit_index);
			if (child != NULL)
				return child;
		}

		return NULL;
	}

	u32 find_unit_index(UnitCompiler &c, Guid unit_id, u32 root_unit_index)
	{
		Unit *root = find_root_unit(c, root_unit_index);
		if (root != NULL) {
			auto cur = hash_map::begin(c._units);
			auto end = hash_map::end(c._units);
			for (; cur != end; ++cur) {
				HASH_MAP_SKIP_HOLE(c._units, cur);

				if (cur->second != root)
					continue;

				if (cur->first == unit_id) {
					return cur->second->_index;
				}

				const u32 child_index = find_child_unit_index(root, unit_id);
				if (child_index != UINT32_MAX)
					return child_index;

				break;
			}
		}

		auto cur = hash_map::begin(c._units);
		auto end = hash_map::end(c._units);
		for (; cur != end; ++cur) {
			HASH_MAP_SKIP_HOLE(c._units, cur);

			if (cur->first == unit_id)
				return cur->second->_index;

			const u32 child_index = find_child_unit_index(cur->second, unit_id);
			if (child_index != UINT32_MAX)
				return child_index;
		}

		return UINT32_MAX;
	}

	Unit *find_unit(UnitCompiler &c, u32 unit_index)
	{
		auto cur = hash_map::begin(c._units);
		auto end = hash_map::end(c._units);
		for (; cur != end; ++cur) {
			HASH_MAP_SKIP_HOLE(c._units, cur);

			Unit *unit = find_unit_by_index(cur->second, unit_index);
			if (unit != NULL)
				return unit;
		}

		return NULL;
	}

	s32 unit_has_component_type(bool &has_type, Unit *unit, StringId32 type)
	{
		CE_ENSURE(unit != NULL);
		has_type = false;

		for (u32 i = 0; i < array::size(unit->_merged_components); ++i) {
			TempAllocator512 ta;
			JsonObject component(ta);
			RETURN_IF_ERROR(sjson::parse(component, unit->_merged_components[i]));

			StringId32 component_type;
			if (json_object::has(component, "_type")) {
				component_type = RETURN_IF_ERROR(sjson::parse_string_id(component["_type"]));
			} else {
				component_type = RETURN_IF_ERROR(sjson::parse_string_id(component["type"]));
			}

			if (component_type == type) {
				has_type = true;
				return 0;
			}
		}

		return 0;
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

	void to_flat(FlatJsonObject &obj, const char *json_object, DynamicString &prefix)
	{
		TempAllocator4096 ta;
		JsonObject jo(ta);
		sjson::parse_object(jo, json_object);

		auto cur = json_object::begin(jo);
		auto end = json_object::end(jo);
		for (; cur != end; ++cur) {
			JSON_OBJECT_SKIP_HOLE(jo, cur);

			DynamicString key(default_allocator());
			key = prefix;
			if (key.length() != 0)
				key += ".";
			key += cur->first;

			if (json::type(cur->second) == JsonValueType::OBJECT) {
				hash_map::set(obj, key, cur->second);
				DynamicString new_prefix(default_allocator());
				to_flat(obj, cur->second, key);
			} else {
				hash_map::set(obj, key, cur->second);
			}
		}
	}

	s32 modify_unit_components(Unit *unit, const char *unit_json, CompileOptions &opts)
	{
		TempAllocator4096 ta;
		JsonObject obj(ta);
		RETURN_IF_ERROR(sjson::parse(obj, unit_json));

		if (json_object::has(obj, "components")) {
			JsonArray components(ta);
			RETURN_IF_ERROR(sjson::parse_array(components, obj["components"]));

			// Add components.
			for (u32 cc = 0; cc < array::size(components); ++cc) {
				FlatJsonObject flat_comp(default_allocator());
				DynamicString empty(default_allocator());
				to_flat(flat_comp, components[cc], empty);

				array::push_back(unit->_merged_components, components[cc]);
				vector::push_back(unit->_flattened_components, flat_comp);
			}
		}

		if (json_object::has(obj, "deleted_components")) {
			JsonObject deleted_components(ta);
			RETURN_IF_ERROR(sjson::parse_object(deleted_components, obj["deleted_components"]));

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

				u32 comp_idx = object_index(unit->_merged_components, component_id, opts);
				if (comp_idx != UINT32_MAX) {
					u32 comp_last = array::size(unit->_merged_components) - 1;
					if (comp_idx != comp_last) {
						unit->_merged_components[comp_idx] = unit->_merged_components[comp_last];
						unit->_flattened_components[comp_idx] = unit->_flattened_components[comp_last];
					}
					array::pop_back(unit->_merged_components);
					vector::pop_back(unit->_flattened_components);
				} else {
					char buf[GUID_BUF_LEN];
					RETURN_IF_FALSE(false
						, opts
						, "Deletion of unexisting component ID: %s"
						, guid::to_string(buf, sizeof(buf), component_id)
						);
				}
			}
		}

		if (json_object::has(obj, "modified_components")) {
			JsonObject modified_components(ta);
			RETURN_IF_ERROR(sjson::parse(modified_components, obj["modified_components"]));

			// Modify components.
			auto cur = json_object::begin(modified_components);
			auto end = json_object::end(modified_components);
			for (; cur != end; ++cur) {
				JSON_OBJECT_SKIP_HOLE(modified_components, cur);

				// Extract GUID from key "#xxxxxxxx-xxxx-xxxx-xxxx-xxxxxxxxxxxx".
				char guid[37];
				strncpy(guid, cur->first.data() + 1, sizeof(guid) - 1);
				guid[36] = '\0';
				Guid component_id = guid::parse(guid);

				// Patch flattened component's keys.
				u32 comp_idx = object_index(unit->_merged_components, component_id, opts);
				if (comp_idx != UINT32_MAX) {
					DynamicString empty(default_allocator());
					to_flat(unit->_flattened_components[comp_idx], cur->second, empty);
				} else {
					char buf[GUID_BUF_LEN];
					RETURN_IF_FALSE(false
						, opts
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
		, CompileOptions &opts
		)
	{
		TempAllocator4096 ta;
		JsonObject obj(ta);
		RETURN_IF_ERROR(sjson::parse(obj, unit_json));

		Guid id = RETURN_IF_ERROR(sjson::parse_guid(obj["_guid"]));

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

		if (json_object::has(obj, "prefab")
			&& sjson::type(obj["prefab"]) == JsonValueType::STRING) {
			TempAllocator512 ta;
			DynamicString prefab(ta);
			RETURN_IF_ERROR(sjson::parse_string(prefab, obj["prefab"]));
			const char *prefab_json_data = prefab_json(c, prefab.c_str());
			RETURN_IF_FALSE(prefab_json_data != NULL
				, opts
				, "Unknown prefab: '%s'"
				, prefab.c_str()
				);

			s32 err = parse_unit_internal(c
				, prefab_json_data
				, unit
				, NULL
				, opts
				);
			ENSURE_OR_RETURN(err == 0, opts);
		}

		s32 err = modify_unit_components(unit, unit_json, opts);
		ENSURE_OR_RETURN(err == 0, opts);

		if (json_object::has(obj, "children")) {
			JsonArray children(ta);
			RETURN_IF_ERROR(sjson::parse_array(children, obj["children"]));

			for (u32 cc = 0; cc < array::size(children); ++cc) {
				s32 err = parse_unit_internal(c
					, children[cc]
					, NULL
					, unit
					, opts
					);
				ENSURE_OR_RETURN(err == 0, opts);
			}
		}

		if (json_object::has(obj, "deleted_children")) {
			JsonArray deleted_children(ta);
			RETURN_IF_ERROR(sjson::parse_array(deleted_children, obj["deleted_children"]));

			// Delete children.
			for (u32 ii = 0; ii < array::size(deleted_children); ++ii) {
				JsonObject obj(ta);
				RETURN_IF_ERROR(sjson::parse_object(obj, deleted_children[ii]));
				Guid id = RETURN_IF_ERROR(sjson::parse_guid(obj["id"]));

				Unit *child = find_children(unit, id);

				char buf[GUID_BUF_LEN];
				RETURN_IF_FALSE(child != NULL
					, opts
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
			RETURN_IF_ERROR(sjson::parse_array(modified_children, obj["modified_children"]));

			for (u32 ii = 0; ii < array::size(modified_children); ++ii) {
				JsonObject obj(ta);
				RETURN_IF_ERROR(sjson::parse_object(obj, modified_children[ii]));
				Guid id = RETURN_IF_ERROR(sjson::parse_guid(obj["id"]));

				Unit *child = find_children(unit, id);

				char buf[GUID_BUF_LEN];
				RETURN_IF_FALSE(child != NULL
					, opts
					, "Modification of unexisting child ID: %s"
					, guid::to_string(buf, sizeof(buf), id)
					);

				s32 err = modify_unit_components(child, modified_children[ii], opts);
				ENSURE_OR_RETURN(err == 0, opts);
			}
		}

		// Parse unit's editor name.
		if (json_object::has(obj, "editor")) {
			JsonObject editor(ta);
			RETURN_IF_ERROR(sjson::parse(editor, obj["editor"]));

			if (json_object::has(editor, "name")) {
				unit->_editor_name = RETURN_IF_ERROR(sjson::parse_string_id(editor["name"]));
			}
		}

		return 0;
	}

	s32 flatten(UnitCompiler &c, CompileOptions &opts);

	s32 assign_unit_indices(UnitCompiler &c, Unit *unit, u32 parent_unit_index, CompileOptions &opts)
	{
		const u32 unit_index = c._num_units;
		const u32 root_unit_index = parent_unit_index == UINT32_MAX
			? unit_index
			: c._unit_roots[parent_unit_index]
			;
		unit->_index = unit_index;
		array::push_back(c._unit_parents, parent_unit_index);
		array::push_back(c._unit_roots, root_unit_index);
		array::push_back(c._unit_names, unit->_editor_name);
		++c._num_units;

		for (u32 cc = 0; cc < array::size(unit->_merged_components); ++cc) {
			TempAllocator512 ta;
			JsonObject component(ta);
			RETURN_IF_ERROR(sjson::parse(component, unit->_merged_components[cc]));

			Guid component_id;
			if (json_object::has(component, "id")) {
				component_id = RETURN_IF_ERROR(sjson::parse_guid(component["id"]));
			} else {
				component_id = RETURN_IF_ERROR(sjson::parse_guid(component["_guid"]));
			}

			StringId32 comp_type;
			if (!json_object::has(component, "_type")) {
				comp_type = RETURN_IF_ERROR(sjson::parse_string_id(component["type"]));
			} else {
				comp_type = RETURN_IF_ERROR(sjson::parse_string_id(component["_type"]));
			}

			const ComponentKey key = { component_id, root_unit_index };
			hash_map::set(c._component_unit_index, key, unit_index);
			hash_map::set(c._component_type, key, comp_type);
		}

		auto cur = hash_map::begin(unit->_children);
		auto end = hash_map::end(unit->_children);
		for (; cur != end; ++cur) {
			HASH_MAP_SKIP_HOLE(unit->_children, cur);

			s32 err = assign_unit_indices(c, cur->second, unit_index, opts);
			ENSURE_OR_RETURN(err == 0, opts);
		}

		return 0;
	}

	s32 parse_unit_from_json(UnitCompiler &c, const char *unit_json, CompileOptions &opts)
	{
		s32 err = collect_prefabs(c, StringId64(), unit_json, true, opts);
		ENSURE_OR_RETURN(err == 0, opts);

		u32 original_unit = c._prefab_offsets[array::size(c._prefab_offsets) - 1];
		err = parse_unit_internal(c, &c._prefab_data[original_unit], NULL, NULL, opts);
		ENSURE_OR_RETURN(err == 0, opts);

		return flatten(c, opts);
	}

	s32 parse_unit(UnitCompiler &c, const char *path, CompileOptions &opts)
	{
		return parse_unit_from_json(c, array::begin(read_unit(path, opts)), opts);
	}

	s32 parse_unit_array_from_json(UnitCompiler &c, const char *units_array_json, CompileOptions &opts)
	{
		TempAllocator4096 ta;
		JsonArray units(ta);
		RETURN_IF_ERROR(sjson::parse_array(units, units_array_json));

		Array<u32> original_units(default_allocator());

		for (u32 i = 0; i < array::size(units); ++i) {
			s32 err = collect_prefabs(c, StringId64(), units[i], true, opts);
			ENSURE_OR_RETURN(err == 0, opts);
			u32 original_unit = c._prefab_offsets[array::size(c._prefab_offsets) - 1];
			array::push_back(original_units, original_unit);
		}

		for (u32 i = 0; i < array::size(units); ++i) {
			s32 err = parse_unit_internal(c, &c._prefab_data[original_units[i]], NULL, NULL, opts);
			ENSURE_OR_RETURN(err == 0, opts);
		}

		return flatten(c, opts);
	}

	s32 flatten_unit(UnitCompiler &c, Unit *unit, u32 parent_unit_index, CompileOptions &opts)
	{
		CE_UNUSED(parent_unit_index);

		const u32 unit_index = unit->_index;
		bool unit_has_transform = false;

		// Compile component data for each component type found
		// in the tree of units.
		for (u32 cc = 0; cc < array::size(unit->_merged_components); ++cc) {
			const char *component_json = unit->_merged_components[cc];

			TempAllocator512 ta;
			JsonObject component(ta);
			RETURN_IF_ERROR(sjson::parse(component, component_json));

			StringId32 comp_type;
			if (!json_object::has(component, "_type")) {
				comp_type = RETURN_IF_ERROR(sjson::parse_string_id(component["type"]));
				logw(UNIT_COMPILER, "'type' property is deprecated: replace with equivalent '_type'");
			} else {
				comp_type = RETURN_IF_ERROR(sjson::parse_string_id(component["_type"]));
			}

			if (comp_type == STRING_ID_32("transform", UINT32_C(0xad9b5315)))
				unit_has_transform = true;

			// Append data to the component data for the given type.
			ComponentTypeData ctd_deffault(default_allocator());
			ComponentTypeData &ctd = const_cast<ComponentTypeData &>(hash_map::get(c._component_data, comp_type, ctd_deffault));
			RETURN_IF_FALSE(&ctd != &ctd_deffault, opts, "Unknown component type");

			// Compile component.
			Buffer comp_data(default_allocator());
			c._current_unit_index = unit_index;
			s32 err = ctd._compiler(comp_data, c, unit->_flattened_components[cc], opts);
			ENSURE_OR_RETURN(err == 0, opts);

			// One component per unit max.
			auto cur = array::begin(ctd._unit_index);
			auto end = array::end(ctd._unit_index);
			if (std::find(cur, end, unit_index) != end) {
				char buf[STRING_ID32_BUF_LEN];
				RETURN_IF_FALSE(false
					, opts
					, "Unit already has a component of type: %s"
					, comp_type.to_string(buf, sizeof(buf))
					);
			}

			array::push(ctd._data, array::begin(comp_data), array::size(comp_data));
			array::push_back(ctd._unit_index, unit_index);
			++ctd._num;
		}

		// Flatten children tree.
		auto cur = hash_map::begin(unit->_children);
		auto end = hash_map::end(unit->_children);
		for (; cur != end; ++cur) {
			HASH_MAP_SKIP_HOLE(unit->_children, cur);

			RETURN_IF_FALSE(unit_has_transform
				, opts
				, "Units with children must have 'transform' component"
				);
			s32 err = flatten_unit(c, cur->second, unit_index, opts);
			ENSURE_OR_RETURN(err == 0, opts);
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

	s32 flatten(UnitCompiler &c, CompileOptions &opts)
	{
		auto cur = hash_map::begin(c._units);
		auto end = hash_map::end(c._units);
		for (; cur != end; ++cur) {
			HASH_MAP_SKIP_HOLE(c._units, cur);

			s32 err = assign_unit_indices(c, cur->second, UINT32_MAX, opts);
			ENSURE_OR_RETURN(err == 0, opts);
		}

		cur = hash_map::begin(c._units);
		end = hash_map::end(c._units);
		for (; cur != end; ++cur) {
			HASH_MAP_SKIP_HOLE(c._units, cur);

			s32 err = flatten_unit(c, cur->second, UINT32_MAX, opts);
			ENSURE_OR_RETURN(err == 0, opts);
		}

		return 0;
	}

	s32 blob(Buffer &output, UnitCompiler &c)
	{
		FileBuffer fb(output);
		BinaryWriter bw(fb);

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
	: _index(UINT32_MAX)
	, _merged_components(a)
	, _flattened_components(a)
	, _children(a)
	, _parent(NULL)
{
}

UnitCompiler::UnitCompiler(Allocator &a)
	: _units(a)
	, _prefab_data(a)
	, _prefab_offsets(a)
	, _prefab_names(a)
	, _component_data(a)
	, _component_unit_index(a)
	, _component_type(a)
	, _component_info(a)
	, _unit_names(a)
	, _unit_parents(a)
	, _unit_roots(a)
	, _current_unit_index(UINT32_MAX)
	, _num_units(0)
{
	unit_compiler::register_component_compiler(*this, "transform",               &compile_transform,                                  0.0f);
	unit_compiler::register_component_compiler(*this, "camera",                  &compile_camera,                                     1.0f);
	unit_compiler::register_component_compiler(*this, "mesh_renderer",           &compile_mesh_renderer,                              1.0f);
	unit_compiler::register_component_compiler(*this, "sprite_renderer",         &compile_sprite_renderer,                            1.0f);
	unit_compiler::register_component_compiler(*this, "light",                   &compile_light,                                      1.0f);
	unit_compiler::register_component_compiler(*this, "lod_group",               &compile_lod_group,                                  2.0f);
	unit_compiler::register_component_compiler(*this, "script",                  &compile_script,                                     1.0f);
	unit_compiler::register_component_compiler(*this, "collider",                &physics_resource_internal::compile_collider,        1.0f);
	unit_compiler::register_component_compiler(*this, "actor",                   &physics_resource_internal::compile_actor,           2.0f);
	unit_compiler::register_component_compiler(*this, "mover",                   &physics_resource_internal::compile_mover,           2.0f);
	unit_compiler::register_component_compiler(*this, "fixed_joint",             &physics_resource_internal::compile_fixed_joint,     3.0f);
	unit_compiler::register_component_compiler(*this, "hinge_joint",             &physics_resource_internal::compile_hinge_joint,     3.0f);
	unit_compiler::register_component_compiler(*this, "spherical_joint",         &physics_resource_internal::compile_spherical_joint, 3.0f);
	unit_compiler::register_component_compiler(*this, "limb_joint",              &physics_resource_internal::compile_limb_joint,      3.0f);
	unit_compiler::register_component_compiler(*this, "spring_joint",            &physics_resource_internal::compile_spring_joint,    3.0f);
	unit_compiler::register_component_compiler(*this, "d6_joint",                &physics_resource_internal::compile_d6_joint,        3.0f);
	unit_compiler::register_component_compiler(*this, "animation_state_machine", &compile_animation_state_machine,                    3.0f);
	unit_compiler::register_component_compiler(*this, "fog",                     &compile_fog,                                        0.0f);
	unit_compiler::register_component_compiler(*this, "global_lighting",         &compile_global_lighting,                            0.0f);
	unit_compiler::register_component_compiler(*this, "bloom",                   &compile_bloom,                                      0.0f);
	unit_compiler::register_component_compiler(*this, "tonemap",                 &compile_tonemap,                                    0.0f);
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
