/*
 * Copyright (c) 2012-2026 Daniele Bartolini et al.
 * SPDX-License-Identifier: MIT
 */

#include "config.h"

#if CROWN_CAN_COMPILE
#include "core/containers/array.inl"
#include "core/json/json_object.inl"
#include "core/json/sjson.h"
#include "core/memory/temp_allocator.inl"
#include "core/strings/dynamic_string.inl"
#include "core/strings/string_id.inl"
#include "core/strings/string.inl"
#include "device/log.h"
#include "resource/compile_options.inl"
#include "resource/stat_config_resource.h"
#include "resource/types.h"
#include <algorithm> // std::sort
#include <ctype.h>   // isspace
#include <stdlib.h>  // strtod
#include <string.h>  // strncpy

LOG_SYSTEM(STAT_CONFIG_RESOURCE, "stat_config_resource")

namespace crown
{
namespace stat_config_resource_internal
{
	struct OrderedCounter
	{
		f32 order;
		StatCounter counter;
	};

	struct OrderedPanel
	{
		f32 order;
		StatPanel panel;
	};

	static bool parse_fraction(f32 &value, const char *str)
	{
		const char *cur = skip_spaces(str);
		char *end;
		const f64 numerator = strtod(cur, &end);
		if (end == cur)
			return false;

		cur = skip_spaces(end);
		f64 result = numerator;
		if (*cur == '/') {
			cur = skip_spaces(cur + 1);
			const f64 denominator = strtod(cur, &end);
			if (end == cur || denominator == 0.0)
				return false;
			result /= denominator;
			cur = skip_spaces(end);
		}

		if (*cur != '\0' || !(result >= 0.0 && result <= 1.0))
			return false;

		value = (f32)result;
		return true;
	}

	static s32 parse_margin(f32 &value, const char *json, CompileOptions &opts)
	{
		bool valid;
		if (sjson::type(json) == JsonValueType::STRING) {
			TempAllocator256 ta;
			DynamicString fraction(ta);
			RETURN_IF_ERROR(sjson::parse_string(fraction, json));
			valid = parse_fraction(value, fraction.c_str());
		} else {
			value = RETURN_IF_ERROR(sjson::parse_float(json));
			valid = value >= 0.0f && value <= 1.0f;
		}

		RETURN_IF_FALSE(STAT_CONFIG_RESOURCE
			, valid
			, opts
			, "Invalid stat panel dimensions"
			);
		return 0;
	}

	s32 compile(CompileOptions &opts)
	{
		Buffer buf = opts.read();
		TempAllocator4096 ta;
		JsonObject obj(ta);
		JsonArray panel_objects(ta);
		RETURN_IF_ERROR(sjson::parse(obj, buf));

		RETURN_IF_ERROR(sjson::parse_array(panel_objects, obj["panels"]));
		enum { MAX_PANELS = 32 };
		const u32 num_panels = array::size(panel_objects);
		RETURN_IF_FALSE(STAT_CONFIG_RESOURCE, num_panels <= MAX_PANELS, opts, "Too many stat panels");

		OrderedPanel panels[MAX_PANELS] = {};

		StatConfigResource config;
		config.version = RESOURCE_HEADER(RESOURCE_VERSION_STAT_CONFIG);
		config.num_panels = num_panels;

		for (u32 i = 0; i < config.num_panels; ++i) {
			TempAllocator4096 ta;
			JsonObject panel_obj(ta);
			JsonArray counters(ta);
			DynamicString font(ta);
			DynamicString material(ta);
			DynamicString label(ta);
			DynamicString name(ta);

			StatPanel &panel = panels[i].panel;
			RETURN_IF_ERROR(sjson::parse(panel_obj, panel_objects[i]));

			RETURN_IF_ERROR(sjson::parse_string(name, panel_obj["name"]));
			strncpy(panel.name, name.c_str(), sizeof(panel.name) - 1);

			RETURN_IF_ERROR(sjson::parse_string(font, panel_obj["font"]));
			RETURN_IF_ERROR(sjson::parse_string(material, panel_obj["material"]));
			RETURN_IF_MISSING(STAT_CONFIG_RESOURCE, "font", font.c_str(), opts);
			RETURN_IF_MISSING(STAT_CONFIG_RESOURCE, "material", material.c_str(), opts);
			opts.add_requirement("font", font.c_str());
			opts.add_requirement("material", material.c_str());
			panel.font = StringId64(font.c_str());
			panel.material = StringId64(material.c_str());

			panel.font_size = 16;
			if (json_object::has(panel_obj, "font_size")) {
				const s32 font_size = RETURN_IF_ERROR(sjson::parse_int(panel_obj["font_size"]));
				RETURN_IF_FALSE(STAT_CONFIG_RESOURCE, font_size > 0, opts, "Invalid stat panel dimensions");
				panel.font_size = (u32)font_size;
			}

			RETURN_IF_ERROR(sjson::parse_string(label, panel_obj["label"]));
			strncpy(panel.label, label.c_str(), sizeof(panel.label) - 1);

			JsonObject margin(ta);
			JsonObject padding(ta);
			RETURN_IF_ERROR(sjson::parse_object(margin, panel_obj["margin"]));
			RETURN_IF_ERROR(sjson::parse_object(padding, panel_obj["padding"]));
			if (parse_margin(panel.layout.margin_left, margin["left"], opts) != 0
				|| parse_margin(panel.layout.margin_right, margin["right"], opts) != 0
				|| parse_margin(panel.layout.margin_bottom, margin["bottom"], opts) != 0
				|| parse_margin(panel.layout.margin_top, margin["top"], opts) != 0
				)
				return -1;
			panel.layout.padding_left   = RETURN_IF_ERROR(sjson::parse_float(padding["left"]));
			panel.layout.padding_right  = RETURN_IF_ERROR(sjson::parse_float(padding["right"]));
			panel.layout.padding_bottom = RETURN_IF_ERROR(sjson::parse_float(padding["bottom"]));
			panel.layout.padding_top    = RETURN_IF_ERROR(sjson::parse_float(padding["top"]));

			RETURN_IF_FALSE(STAT_CONFIG_RESOURCE
				, panel.layout.margin_left + panel.layout.margin_right < 1.0f
				&& panel.layout.margin_bottom + panel.layout.margin_top < 1.0f
				&& panel.layout.padding_left >= 0.0f
				&& panel.layout.padding_right >= 0.0f
				&& panel.layout.padding_bottom >= 0.0f
				&& panel.layout.padding_top >= 0.0f
				, opts
				, "Invalid stat panel dimensions"
				);
			if (json_object::has(panel_obj, "order")) {
				panels[i].order = RETURN_IF_ERROR(sjson::parse_float(panel_obj["order"]));
			}

			RETURN_IF_ERROR(sjson::parse_array(counters, panel_obj["counters"]));
			const u32 num_counters = array::size(counters);
			RETURN_IF_FALSE(STAT_CONFIG_RESOURCE, num_counters <= countof(panel.counters), opts, "Too many stat counters");
			panel.num_counters = num_counters;

			OrderedCounter ordered_counters[countof(panel.counters)] = {};
			for (u32 j = 0; j < panel.num_counters; ++j) {
				TempAllocator2048 ta;
				JsonObject counter_obj(ta);
				DynamicString counter_label(ta);
				DynamicString counter_names(ta);

				StatCounter &counter = ordered_counters[j].counter;
				RETURN_IF_ERROR(sjson::parse(counter_obj, counters[j]));
				RETURN_IF_ERROR(sjson::parse_string(counter_label, counter_obj["label"]));
				strncpy(counter.label, counter_label.c_str(), sizeof(counter.label) - 1);
				RETURN_IF_ERROR(sjson::parse_string(counter_names, counter_obj["counters"]));

				if (json_object::has(counter_obj, "order")) {
					ordered_counters[j].order = RETURN_IF_ERROR(sjson::parse_float(counter_obj["order"]));
				}

				const char *cur = counter_names.c_str();
				while (*cur != '\0') {
					cur = skip_spaces(cur);
					if (*cur == '\0')
						break;

					const char *end = cur;
					while (*end != '\0' && !isspace((u8)*end)) ++end;

					RETURN_IF_FALSE(STAT_CONFIG_RESOURCE, counter.num_counters < countof(counter.counters), opts, "Too many stat counter names");
					counter.counters[counter.num_counters++] = StringId32(cur, u32(end - cur))._id;
					cur = end;
				}
			}

			std::sort(ordered_counters
				, ordered_counters + panel.num_counters
				, [](const OrderedCounter &a, const OrderedCounter &b) {
					return a.order < b.order;
				}
				);
			for (u32 j = 0; j < panel.num_counters; ++j)
				panel.counters[j] = ordered_counters[j].counter;
		}

		std::sort(panels
			, panels + config.num_panels
			, [](const OrderedPanel &a, const OrderedPanel &b) {
				return a.order < b.order;
			}
			);

		opts.write(config);
		for (u32 i = 0; i < config.num_panels; ++i)
			opts.write(panels[i].panel);

		return 0;
	}

} // namespace stat_config_resource_internal

} // namespace crown

#endif // if CROWN_CAN_COMPILE
