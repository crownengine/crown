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
#if CROWN_CAN_COMPILE
namespace render_config_resource_internal
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

	s32 parse_render_settings(RenderSettings &rs, const char *settings_json, CompileOptions &opts)
	{
		TempAllocator1024 ta;
		JsonObject obj(ta);
		RETURN_IF_ERROR(sjson::parse(obj, settings_json), opts);

		auto cur = json_object::begin(obj);
		auto end = json_object::end(obj);
		for (; cur != end; ++cur) {
			JSON_OBJECT_SKIP_HOLE(obj, cur);

			if (cur->first == "sun_shadow_map_size") {
				rs.sun_shadow_map_size = RETURN_IF_ERROR(sjson::parse_vector2(cur->second), opts);
			} else if (cur->first == "sun_shadows") {
				bool en = RETURN_IF_ERROR(sjson::parse_bool(cur->second), opts);
				if (en)
					rs.flags |= RenderSettingsFlags::SUN_SHADOWS;
				else
					rs.flags &= ~RenderSettingsFlags::SUN_SHADOWS;
			} else if (cur->first == "local_lights_shadow_map_size") {
				rs.local_lights_shadow_map_size = RETURN_IF_ERROR(sjson::parse_vector2(cur->second), opts);
			} else if (cur->first == "local_lights") {
				bool en = RETURN_IF_ERROR(sjson::parse_bool(cur->second), opts);
				if (en)
					rs.flags |= RenderSettingsFlags::LOCAL_LIGHTS;
				else
					rs.flags &= ~RenderSettingsFlags::LOCAL_LIGHTS;
			} else if (cur->first == "local_lights_shadows") {
				bool en = RETURN_IF_ERROR(sjson::parse_bool(cur->second), opts);
				if (en)
					rs.flags |= RenderSettingsFlags::LOCAL_LIGHTS_SHADOWS;
				else
					rs.flags &= ~RenderSettingsFlags::LOCAL_LIGHTS_SHADOWS;
			} else if (cur->first == "local_lights_distance_culling") {
				bool en = RETURN_IF_ERROR(sjson::parse_bool(cur->second), opts);
				if (en)
					rs.flags |= RenderSettingsFlags::LOCAL_LIGHTS_DISTANCE_CULLING;
				else
					rs.flags &= ~RenderSettingsFlags::LOCAL_LIGHTS_DISTANCE_CULLING;
			} else if (cur->first == "bloom") {
				bool en = RETURN_IF_ERROR(sjson::parse_bool(cur->second), opts);
				if (en)
					rs.flags |= RenderSettingsFlags::BLOOM;
				else
					rs.flags &= ~RenderSettingsFlags::BLOOM;
			} else if (cur->first == "msaa") {
				bool en = RETURN_IF_ERROR(sjson::parse_bool(cur->second), opts);
				if (en)
					rs.flags |= RenderSettingsFlags::MSAA;
				else
					rs.flags &= ~RenderSettingsFlags::MSAA;
			} else if (cur->first == "msaa_quality") {
				StringId32 quality = RETURN_IF_ERROR(sjson::parse_string_id(cur->second), opts);
				rs.msaa_quality = msaa_quality_samples(quality);
			} else if (cur->first == "local_lights_distance_culling_fade") {
				rs.local_lights_distance_culling_fade = RETURN_IF_ERROR(sjson::parse_float(cur->second), opts);
			} else if (cur->first == "local_lights_distance_culling_cutoff") {
				rs.local_lights_distance_culling_cutoff = RETURN_IF_ERROR(sjson::parse_float(cur->second), opts);
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

	s32 compile(CompileOptions &opts)
	{
		Buffer buf = opts.read();
		TempAllocator4096 ta;
		JsonObject obj(ta);
		RETURN_IF_ERROR(sjson::parse(obj, buf), opts);

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

		// Parse.
		if (json_object::has(obj, "render_settings")) {
			s32 err = parse_render_settings(rcr.render_settings, obj["render_settings"], opts);
			ENSURE_OR_RETURN(err == 0, opts);
		}

		// Write.
		opts.write(rcr.version);
		opts.write(rcr.render_settings.flags);
		opts.write(rcr.render_settings.sun_shadow_map_size);
		opts.write(rcr.render_settings.local_lights_shadow_map_size);
		opts.write(rcr.render_settings.local_lights_distance_culling_fade);
		opts.write(rcr.render_settings.local_lights_distance_culling_cutoff);
		opts.write(rcr.render_settings.msaa_quality);

		return 0;
	}

} // namespace render_config_resource_internal
#endif // if CROWN_CAN_COMPILE

} // namespace crown
