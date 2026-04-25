/*
 * Copyright (c) 2012-2026 Daniele Bartolini et al.
 * SPDX-License-Identifier: MIT
 */

#include "config.h"
#include "core/json/json_object.inl"
#include "core/json/sjson.h"
#include "core/memory/temp_allocator.inl"
#include "core/platform.h"
#include "core/strings/dynamic_string.inl"
#include "core/strings/string_id.inl"
#include "device/boot_config.h"
#include "device/log.h"
#include "resource/render_config_resource.h"

LOG_SYSTEM(BOOT_CONFIG, "boot_config")

namespace crown
{
BootConfig::BootConfig(Allocator &a)
	: boot_script_name(a)
	, boot_package_name(u64(0))
	, render_config_name(STRING_ID_64("core/renderer/default", UINT64_C (0x1b92f3ff7ca4157c)))
	, window_title(a)
	, save_dir(a)
	, user_config(a)
	, renderer_type(RendererType::AUTO)
	, window_w(CROWN_DEFAULT_WINDOW_WIDTH)
	, window_h(CROWN_DEFAULT_WINDOW_HEIGHT)
	, device_id(0)
	, aspect_ratio(-1.0f)
	, vsync(true)
	, fullscreen(false)
	, physics_settings({ 60, 4, 10, 0.5f })
	, render_settings(a)
{
}

RendererType::Enum renderer_type_from_name(const char *name)
{
	if (strcmp(name, "auto") == 0)
		return RendererType::AUTO;
	else if (strcmp(name, "d3d11") == 0)
		return RendererType::DIRECT3D11;
	else if (strcmp(name, "gl") == 0)
		return RendererType::OPENGL;
	else if (strcmp(name, "gles") == 0)
		return RendererType::OPENGLES;
	else if (strcmp(name, "vk") == 0)
		return RendererType::VULKAN;

	return RendererType::COUNT;
}

static void parse_physics(PhysicsSettings *settings, const char *json)
{
	TempAllocator1024 ta;
	JsonObject obj(ta);
	sjson::parse(obj, json);

	auto cur = json_object::begin(obj);
	auto end = json_object::end(obj);
	for (; cur != end; ++cur) {
		JSON_OBJECT_SKIP_HOLE(obj, cur);

		if (cur->first == "step_frequency") {
			settings->step_frequency = sjson::parse_int(cur->second);
		} else if (cur->first == "max_substeps") {
			settings->max_substeps = sjson::parse_int(cur->second);
		} else if (cur->first == "solver_iterations") {
			settings->solver_iterations = sjson::parse_int(cur->second);
		} else if (cur->first == "sleep_threshold") {
			settings->sleep_threshold = sjson::parse_float(cur->second);
		} else {
			logw(BOOT_CONFIG
				, "Unknown physics property '%.*s'"
				, cur->first.length()
				, cur->first.data()
				);
		}
	}
}

static void parse_renderer_settings(BootConfig *config, const char *json)
{
	TempAllocator1024 ta;
	JsonObject renderer(ta);
	sjson::parse(renderer, json);

	auto cur = json_object::begin(renderer);
	auto end = json_object::end(renderer);
	for (; cur != end; ++cur) {
		JSON_OBJECT_SKIP_HOLE(renderer, cur);

		if (cur->first == "resolution") {
			JsonArray resolution(ta);
			sjson::parse_array(resolution, cur->second);
			config->window_w = sjson::parse_int(resolution[0]);
			config->window_h = sjson::parse_int(resolution[1]);
		} else if (cur->first == "aspect_ratio") {
			config->aspect_ratio = sjson::parse_float(cur->second);
		} else if (cur->first == "device_id") {
			DynamicString hex(ta);
			sjson::parse_string(hex, cur->second);
			s64 id;
			from_hex(id, hex.c_str());
			config->device_id = (u16)id;
		} else if (cur->first == "type") {
			DynamicString renderer_type(ta);
			sjson::parse_string(renderer_type, cur->second);
			config->renderer_type = renderer_type_from_name(renderer_type.c_str());
			if (config->renderer_type == RendererType::COUNT) {
				logw(BOOT_CONFIG
					, "Unknown renderer type '%s'"
					, renderer_type.c_str()
					);
				config->renderer_type = RendererType::AUTO;
			}
		} else if (cur->first == "vsync") {
			config->vsync = sjson::parse_bool(cur->second);
		} else if (cur->first == "fullscreen") {
			config->fullscreen = sjson::parse_bool(cur->second);
		} else {
			logw(BOOT_CONFIG
				, "Unknown renderer property '%.*s'"
				, cur->first.length()
				, cur->first.data()
				);
		}
	}
}

static void parse_platform_settings(BootConfig *config, const char *json)
{
	TempAllocator4096 ta;
	JsonObject platform(ta);
	sjson::parse(platform, json);

	auto cur = json_object::begin(platform);
	auto end = json_object::end(platform);
	for (; cur != end; ++cur) {
		JSON_OBJECT_SKIP_HOLE(platform, cur);

		if (cur->first == "save_dir") {
			sjson::parse_string(config->save_dir, cur->second);
		} else if (cur->first == "renderer") {
			parse_renderer_settings(config, cur->second);
		} else if (cur->first == "render_settings") {
			render_settings::parse(config->render_settings, cur->second);
		} else {
			logw(BOOT_CONFIG
				, "Unknown platform property '%.*s'"
				, cur->first.length()
				, cur->first.data()
				);
		}
	}
}

static bool is_platform_name(const StringView &key)
{
	return key == "android"
		|| key == "html5"
		|| key == "ios"
		|| key == "linux"
		|| key == "osx"
		|| key == "windows"
		;
}

bool BootConfig::parse(const char *json)
{
	TempAllocator4096 ta;
	JsonObject cfg(ta);
	sjson::parse(cfg, json);

	auto cur = json_object::begin(cfg);
	auto end = json_object::end(cfg);
	for (; cur != end; ++cur) {
		JSON_OBJECT_SKIP_HOLE(cfg, cur);

		if (cur->first == "boot_script") {
			sjson::parse_string(boot_script_name, cur->second);
		} else if (cur->first == "boot_package") {
			boot_package_name = sjson::parse_resource_name(cur->second);
		} else if (cur->first == "window_title") {
			sjson::parse_string(window_title, cur->second);
		} else if (cur->first == "render_config") {
			render_config_name = sjson::parse_resource_name(cur->second);
		} else if (cur->first == "physics") {
			parse_physics(&physics_settings, cur->second);
		} else if (cur->first == "render_settings") {
			render_settings::parse(render_settings, cur->second);
		} else if (cur->first == "user_config") {
			sjson::parse_string(user_config, cur->second);
		} else if (cur->first == CROWN_PLATFORM_NAME) {
			parse_platform_settings(this, cur->second);
		} else {
			if (!is_platform_name(cur->first)) {
				logw(BOOT_CONFIG
					, "Unknown boot_config property '%.*s'"
					, cur->first.length()
					, cur->first.data()
					);
			}
		}
	}

	return true;
}

} // namespace crown
