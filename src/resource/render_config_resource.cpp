/*
 * Copyright (c) 2012-2026 Daniele Bartolini et al.
 * SPDX-License-Identifier: MIT
 */

#include "config.h"
#include "core/containers/hash_map.inl"
#include "core/json/json_object.inl"
#include "core/json/sjson.h"
#include "core/strings/string.inl"
#include "core/strings/string_id.inl"
#include "resource/compile_options.inl"
#include "resource/render_config_resource.h"
#include "world/types.h"
#include "device/log.h"

LOG_SYSTEM(RENDER_CONFIG_RESOURCE, "render_config_resource")

namespace crown
{
namespace
{
	u32 msaa_quality_samples(StringId32 quality)
	{
		if (quality == STRING_ID_32("low", UINT32_C(0x8876b146)))
			return 1;
		else if (quality == STRING_ID_32("medium", UINT32_C(0x6e03a970)))
			return 2;
		else if (quality == STRING_ID_32("high", UINT32_C(0xfc9141f5)))
			return 3;
		else if (quality == STRING_ID_32("ultra", UINT32_C(0xf13839af)))
			return 4;
		else
			logw(RENDER_CONFIG_RESOURCE, "Unknown msaa quality");

		return 0;
	}

} // namespace

namespace render_settings
{
	s32 parse(HashMap<StringId32, Value> &rs, const char *settings_json)
	{
		TempAllocator1024 ta;
		JsonObject obj(ta);
		RETURN_IF_ERROR(sjson::parse(obj, settings_json));

		auto cur = json_object::begin(obj);
		auto end = json_object::end(obj);
		for (; cur != end; ++cur) {
			JSON_OBJECT_SKIP_HOLE(obj, cur);

			if (cur->first == "sun_shadow_map_size") {
				Value v; v.type = Value::VECTOR2; v.value.v2 = RETURN_IF_ERROR(sjson::parse_vector2(cur->second));
				hash_map::set(rs, cur->first.to_string_id(), v);
			} else if (cur->first == "sun_shadows") {
				Value v; v.type = Value::BOOL; v.value.b = RETURN_IF_ERROR(sjson::parse_bool(cur->second));
				hash_map::set(rs, cur->first.to_string_id(), v);
			} else if (cur->first == "local_lights_shadow_map_size") {
				Value v; v.type = Value::VECTOR2; v.value.v2 = RETURN_IF_ERROR(sjson::parse_vector2(cur->second));
				hash_map::set(rs, cur->first.to_string_id(), v);
			} else if (cur->first == "local_lights") {
				Value v; v.type = Value::BOOL; v.value.b = RETURN_IF_ERROR(sjson::parse_bool(cur->second));
				hash_map::set(rs, cur->first.to_string_id(), v);
			} else if (cur->first == "local_lights_shadows") {
				Value v; v.type = Value::BOOL; v.value.b = RETURN_IF_ERROR(sjson::parse_bool(cur->second));
				hash_map::set(rs, cur->first.to_string_id(), v);
			} else if (cur->first == "local_lights_distance_culling") {
				Value v; v.type = Value::BOOL; v.value.b = RETURN_IF_ERROR(sjson::parse_bool(cur->second));
				hash_map::set(rs, cur->first.to_string_id(), v);
			} else if (cur->first == "bloom") {
				Value v; v.type = Value::BOOL; v.value.b = RETURN_IF_ERROR(sjson::parse_bool(cur->second));
				hash_map::set(rs, cur->first.to_string_id(), v);
			} else if (cur->first == "msaa") {
				Value v; v.type = Value::BOOL; v.value.b = RETURN_IF_ERROR(sjson::parse_bool(cur->second));
				hash_map::set(rs, cur->first.to_string_id(), v);
			} else if (cur->first == "msaa_quality") {
				StringId32 quality = RETURN_IF_ERROR(sjson::parse_string_id(cur->second));
				Value v; v.type = Value::UINT32; v.value.u = msaa_quality_samples(quality);
				hash_map::set(rs, cur->first.to_string_id(), v);
			} else if (cur->first == "local_lights_distance_culling_fade") {
				Value v; v.type = Value::FLOAT; v.value.f = RETURN_IF_ERROR(sjson::parse_float(cur->second));
				hash_map::set(rs, cur->first.to_string_id(), v);
			} else if (cur->first == "local_lights_distance_culling_cutoff") {
				Value v; v.type = Value::FLOAT; v.value.f = RETURN_IF_ERROR(sjson::parse_float(cur->second));
				hash_map::set(rs, cur->first.to_string_id(), v);
			} else if (cur->first == "lod_fade_duration") {
				Value v; v.type = Value::FLOAT; v.value.f = RETURN_IF_ERROR(sjson::parse_float(cur->second));
				hash_map::set(rs, cur->first.to_string_id(), v);
			} else {
				logw(RENDER_CONFIG_RESOURCE
					, "Unknown render_settings property '%.*s'"
					, cur->first.length()
					, cur->first.data()
					);
			}
		}

		return 0;
	}

	static void set_flag(u32 &flags, u32 flag, bool enabled)
	{
		if (enabled)
			flags |= flag;
		else
			flags &= ~flag;
	}

	s32 write(RenderSettings &rs, const HashMap<StringId32, Value> &settings_map)
	{
		auto cur = hash_map::begin(settings_map);
		auto end = hash_map::end(settings_map);
		for (; cur != end; ++cur) {
			HASH_MAP_SKIP_HOLE(settings_map, cur);

			const StringId32 key = cur->first;
			const Value &v = cur->second;

			if (key == STRING_ID_32("sun_shadow_map_size", UINT32_C(0x964ee0ab))) {
				rs.sun_shadow_map_size = v.value.v2;
			} else if (key == STRING_ID_32("sun_shadows", UINT32_C(0x4ef88b61))) {
				set_flag(rs.flags, RenderSettingsFlags::SUN_SHADOWS, v.value.b);
			} else if (key == STRING_ID_32("local_lights_shadow_map_size", UINT32_C(0x28d6b7e9))) {
				rs.local_lights_shadow_map_size = v.value.v2;
			} else if (key == STRING_ID_32("local_lights", UINT32_C(0x831fd434))) {
				set_flag(rs.flags, RenderSettingsFlags::LOCAL_LIGHTS, v.value.b);
			} else if (key == STRING_ID_32("local_lights_shadows", UINT32_C(0x8b47ea20))) {
				set_flag(rs.flags, RenderSettingsFlags::LOCAL_LIGHTS_SHADOWS, v.value.b);
			} else if (key == STRING_ID_32("local_lights_distance_culling", UINT32_C(0x9b7b76bd))) {
				set_flag(rs.flags, RenderSettingsFlags::LOCAL_LIGHTS_DISTANCE_CULLING, v.value.b);
			} else if (key == STRING_ID_32("bloom", UINT32_C(0x995dd31c))) {
				set_flag(rs.flags, RenderSettingsFlags::BLOOM, v.value.b);
			} else if (key == STRING_ID_32("msaa", UINT32_C(0xaab08183))) {
				set_flag(rs.flags, RenderSettingsFlags::MSAA, v.value.b);
			} else if (key == STRING_ID_32("msaa_quality", UINT32_C(0x7464a369))) {
				rs.msaa_quality = v.value.u;
			} else if (key == STRING_ID_32("local_lights_distance_culling_fade", UINT32_C(0xc30a11d4))) {
				rs.local_lights_distance_culling_fade = v.value.f;
			} else if (key == STRING_ID_32("local_lights_distance_culling_cutoff", UINT32_C(0x8fa8d89a))) {
				rs.local_lights_distance_culling_cutoff = v.value.f;
			} else if (key == STRING_ID_32("lod_fade_duration", UINT32_C(0x98ff46dd))) {
				rs.lod_fade_duration = v.value.f;
			} else {
				logw(RENDER_CONFIG_RESOURCE
					, "Unknown render_settings property 0x%08x"
					, key._id
					);
			}
		}

		return 0;
	}

} // namespace render_settings

#if CROWN_CAN_COMPILE
namespace render_config_resource_internal
{
	s32 parse_shaders(const char *shaders_json, CompileOptions &opts)
	{
		TempAllocator1024 ta;
		JsonArray arr(ta);
		RETURN_IF_ERROR(sjson::parse_array(arr, shaders_json));

		for (u32 i = 0; i < array::size(arr); ++i) {
			TempAllocator512 ta;
			DynamicString shader_name(ta);

			RETURN_IF_ERROR(sjson::parse_string(shader_name, arr[i]));
			RETURN_IF_RESOURCE_MISSING("shader", shader_name.c_str(), opts);
			opts.add_requirement("shader", shader_name.c_str());
		}

		return 0;
	}

	s32 compile(CompileOptions &opts)
	{
		Buffer buf = opts.read();
		TempAllocator4096 ta;
		JsonObject obj(ta);
		RETURN_IF_ERROR(sjson::parse(obj, buf));

		// Init defaults.
		RenderConfigResource rcr;
		rcr.version = RESOURCE_HEADER(RESOURCE_VERSION_RENDER_CONFIG);
		rcr.render_settings.flags = 0u
			| RenderSettingsFlags::SUN_SHADOWS
			| RenderSettingsFlags::LOCAL_LIGHTS
			| RenderSettingsFlags::LOCAL_LIGHTS_SHADOWS
			| RenderSettingsFlags::BLOOM
			;
		rcr.render_settings.sun_shadow_map_size = { 4096.0f, 4096.0f };
		rcr.render_settings.local_lights_shadow_map_size = { 2048.0f, 2048.0f };
		rcr.render_settings.local_lights_distance_culling_fade = 30.0f;
		rcr.render_settings.local_lights_distance_culling_cutoff = 60.0f;
		rcr.render_settings.lod_fade_duration = 0.2f;
		rcr.render_settings.msaa_quality = msaa_quality_samples(STRING_ID_32("ultra", UINT32_C(0xf13839af)));

		// Parse.
		if (json_object::has(obj, "render_settings")) {
			TempAllocator1024 ta;
			HashMap<StringId32, Value> render_settings(ta);
			s32 err = render_settings::parse(render_settings, obj["render_settings"]);
			ENSURE_OR_RETURN(err == 0, opts);
			err = render_settings::write(rcr.render_settings, render_settings);
			ENSURE_OR_RETURN(err == 0, opts);
		}

		if (json_object::has(obj, "shaders")) {
			s32 err = parse_shaders(obj["shaders"], opts);
			ENSURE_OR_RETURN(err == 0, opts);
		}

		// Write.
		opts.write(rcr.version);
		opts.write(rcr.render_settings.flags);
		opts.write(rcr.render_settings.sun_shadow_map_size);
		opts.write(rcr.render_settings.local_lights_shadow_map_size);
		opts.write(rcr.render_settings.local_lights_distance_culling_fade);
		opts.write(rcr.render_settings.local_lights_distance_culling_cutoff);
		opts.write(rcr.render_settings.lod_fade_duration);
		opts.write(rcr.render_settings.msaa_quality);

		return 0;
	}

} // namespace render_config_resource_internal
#endif // if CROWN_CAN_COMPILE

} // namespace crown
