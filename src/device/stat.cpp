/*
 * Copyright (c) 2012-2026 Daniele Bartolini et al.
 * SPDX-License-Identifier: MIT
 */

#include "core/containers/array.inl"
#include "core/json/sjson.h"
#include "core/math/constants.h"
#include "core/memory/temp_allocator.inl"
#include "core/profiler.h"
#include "core/strings/dynamic_string.inl"
#include "core/strings/string_id.inl"
#include "device/console_server.h"
#include "device/device.h"
#include "device/log.h"
#include "device/pipeline.h"
#include "device/stat.h"
#include "resource/resource_manager.h"
#include "resource/stat_config_resource.h"
#include "world/gui.h"
#include <stb_sprintf.h>

LOG_SYSTEM(STAT, "stat")

namespace crown
{
namespace stat_internal
{
	struct PanelLayoutRect
	{
		f32 width;
		f32 height;
		f32 left;
		f32 right;
		f32 bottom;
		f32 top;
	};

	struct PanelLayout
	{
		PanelLayoutRect panel;
		PanelLayoutRect content;
	};

	static Allocator *_allocator;
	static ResourceManager *_resource_manager;
	static GuiBuffer *_gui_buffer;
	static Gui *_gui;
	static const StatConfigResource *_config;
	static u32 _enabled_panels;

	static bool calculate_panel_layout(PanelLayout &layout
		, f32 screen_width
		, f32 screen_height
		, const StatPanelLayout &config
		)
	{
		PanelLayoutRect &panel = layout.panel;
		panel.left   = screen_width * config.margin_left;
		panel.right  = screen_width * (1.0f - config.margin_right);
		panel.bottom = screen_height * config.margin_bottom;
		panel.top    = screen_height * (1.0f - config.margin_top);
		panel.width  = panel.right - panel.left;
		panel.height = panel.top - panel.bottom;

		if (config.padding_left + config.padding_right >= panel.width
			|| config.padding_bottom + config.padding_top >= panel.height
			)
			return false;

		PanelLayoutRect &content = layout.content;
		content.left   = panel.left + config.padding_left;
		content.right  = panel.right - config.padding_right;
		content.bottom = panel.bottom + config.padding_bottom;
		content.top    = panel.top - config.padding_top;
		content.width  = content.right - content.left;
		content.height = content.top - content.bottom;
		return true;
	}

	static f32 centered_text_y(f32 row_bottom
		, f32 row_height
		, const char *str
		, u32 font_size
		, StringId64 font
		)
	{
		Vector2 box_min;
		Vector2 box_max;
		_gui->text_extents(box_min, box_max, font_size, str, font);
		const f32 text_height = box_max.y - box_min.y;
		return row_bottom + (row_height - text_height)*0.5f - box_min.y;
	}

	static f32 counter_value(const StatCounter &counter)
	{
		f32 value = 0.0f;
		const char *cur = profiler_globals::buffer_begin();
		const char *end = profiler_globals::buffer_end();
		while (cur != end) {
			const u32 type = *(const u32 *)cur;
			cur += sizeof(u32);
			if (type == ProfilerEventType::COUNT)
				break;

			const u32 size = *(const u32 *)cur;
			cur += sizeof(u32);

			if (type == ProfilerEventType::RECORD_FLOAT) {
				const RecordFloat *rf = (const RecordFloat *)cur;
				const u32 name = StringId32(rf->name)._id;
				for (u32 i = 0; i < counter.num_counters; ++i) {
					if (name == counter.counters[i]) {
						value += rf->value;
						break;
					}
				}
			}

			cur += size;
		}

		return value;
	}

	static void handle_command(ConsoleServer &cs, u32 client_id, const JsonArray &args, void * /*user_data*/)
	{
		TempAllocator1024 ta;
		DynamicString subcmd(ta);
		const u32 num_args = array::size(args);

		if (num_args > 3) {
			cs.error(client_id, "Usage: stat [panel] [on|off]");
			return;
		}

		if (num_args == 1)
			subcmd = "general";
		else
			sjson::parse_string(subcmd, args[1]);

		if (subcmd == "help") {
			logi(STAT, "Usage: stat [panel] [on|off]");
			logi(STAT, "       stat none");
			if (_config != NULL) {
				const StatPanel *panels = (const StatPanel *)(_config + 1);
				for (u32 i = 0; i < _config->num_panels; ++i)
					logi(STAT, "%s %s", panels[i].name, panels[i].label);
			}
			return;
		}

		if (subcmd == "none") {
			if (num_args != 2) {
				cs.error(client_id, "Usage: stat none");
				return;
			}

			_enabled_panels = 0;
			return;
		}

		u32 mask = 0;
		if (_config != NULL) {
			const StatPanel *panels = (const StatPanel *)(_config + 1);
			for (u32 i = 0; i < _config->num_panels; ++i) {
				if (subcmd == panels[i].name) {
					mask = 1u << i;
					break;
				}
			}
		}
		if (mask == 0) {
			cs.error(client_id, "Usage: stat [panel] [on|off]");
			return;
		}

		bool enabled = (_enabled_panels & mask) != 0;
		if (num_args == 3) {
			DynamicString value(ta);
			sjson::parse_string(value, args[2]);
			if (value == "on") {
				enabled = true;
			} else if (value == "off") {
				enabled = false;
			} else {
				cs.error(client_id, "Usage: stat [panel] [on|off]");
				return;
			}
		} else {
			enabled = !enabled;
		}

		if (enabled)
			_enabled_panels |= mask;
		else
			_enabled_panels &= ~mask;
	}

} // namespace stat_internal

namespace stat_globals
{
	void init(Allocator &a
		, ResourceManager &rm
		, ShaderManager &sm
		, MaterialManager &mm
		, Pipeline &pl
		, ConsoleServer &cs
		, const StatConfigResource *config
		)
	{
		using namespace stat_internal;

		_allocator = &a;
		_resource_manager = &rm;
		_config = config;
		_enabled_panels = 0;

		_gui_buffer = CE_NEW(a, GuiBuffer)(sm);
		_gui_buffer->create();
		_gui = gui::create_screen_gui(a
			, *_gui_buffer
			, rm
			, sm
			, mm
			, &pl._gui_shader
			);

		cs.register_command_name("stat", "Show statistics HUD.", stat_internal::handle_command, NULL);
	}

	void shutdown()
	{
		using namespace stat_internal;

		CE_DELETE(*_allocator, _gui);
		CE_DELETE(*_allocator, _gui_buffer);

		_enabled_panels = 0;
		_config = NULL;
		_gui = NULL;
		_gui_buffer = NULL;
		_resource_manager = NULL;
		_allocator = NULL;
	}

	void draw(u16 window_width, u16 window_height)
	{
		using namespace stat_internal;

		if (_enabled_panels == 0 || _config == NULL)
			return;

		_gui_buffer->reset();

		char row[512];
		const f32 title_margin_bottom = 4.0f;
		const Color4 row_colors[] = {
			{ 1.0f, 1.0f, 1.0f, 0.03f },
			{ 1.0f, 1.0f, 1.0f, 0.08f }
		};
		const StatPanel *panels = (const StatPanel *)(_config + 1);

		// Draw enabled panels.
		for (u32 i = 0; i < _config->num_panels; ++i) {
			const StatPanel &panel = panels[i];
			if ((_enabled_panels & (1u << i)) == 0)
				continue;
			if (!_resource_manager->can_get(RESOURCE_TYPE_FONT, panel.font)
				|| !_resource_manager->can_get(RESOURCE_TYPE_MATERIAL, panel.material)
				)
				continue;

			const u32 font_size = panel.font_size;
			const f32 line_height = (f32)font_size + 2.0f;

			// Calculate panel geometry and fit rows to its content area.
			PanelLayout layout;
			if (!calculate_panel_layout(layout
				, (f32)window_width
				, (f32)window_height
				, panel.layout
				))
				continue;

			if (layout.content.height < line_height)
				continue;

			const f32 data_height = layout.content.height - line_height - title_margin_bottom;
			const u32 max_counters = data_height > 0.0f
				? (u32)(data_height / line_height)
				: 0
				;
			const u32 num_counters = min(panel.num_counters, max_counters);
			const f32 title_margin = num_counters > 0 ? title_margin_bottom : 0.0f;
			const f32 content_height = line_height*(1.0f + (f32)num_counters) + title_margin;
			const f32 panel_height_delta = layout.content.height - content_height;
			layout.panel.bottom += panel_height_delta;
			layout.panel.height -= panel_height_delta;
			const f32 data_top = layout.content.top - line_height - title_margin;

			// Draw panel background.
			_gui->rect({ layout.panel.left, layout.panel.bottom, 0.0f }
				, { layout.panel.width, layout.panel.height }
				, { 0.0f, 0.0f, 0.0f, 0.65f }
				);

			// Draw alternating row backgrounds.
			for (u32 j = 0; j < num_counters; ++j) {
				const f32 row_bottom = data_top - line_height*(1.0f + (f32)j);
				_gui->rect({ layout.content.left, row_bottom, 0.0f }
					, { layout.content.width, line_height }
					, row_colors[j % countof(row_colors)]
					);
			}

			// Draw panel title.
			if (panel.label[0] != '\0') {
				const f32 title_y = centered_text_y(layout.content.top - line_height
					, line_height
					, panel.label
					, font_size
					, panel.font
					);
				_gui->text({ layout.content.left, title_y, 0.0f }
					, font_size
					, panel.label
					, panel.font
					, panel.material
					, COLOR4_WHITE
					);
			}

			// Draw counter labels and values.
			for (u32 j = 0; j < num_counters; ++j) {
				const StatCounter &counter = panel.counters[j];
				const f32 row_bottom = data_top - line_height*(1.0f + (f32)j);
				const bool has_value = counter.num_counters != 0;
				f32 value_x = 0.0f;
				f32 value_y = 0.0f;
				if (has_value) {
					stbsp_snprintf(row, sizeof(row), "%.2f", counter_value(counter));
					Vector2 box_min;
					Vector2 box_max;
					const f32 value_width = _gui->text_extents(box_min, box_max, font_size, row, panel.font).x;
					value_x = layout.content.right - value_width;
					value_y = centered_text_y(row_bottom
						, line_height
						, row
						, font_size
						, panel.font
						);
				}

				if (counter.label[0] != '\0') {
					const f32 label_y = centered_text_y(row_bottom
						, line_height
						, counter.label
						, font_size
						, panel.font
						);
					_gui->text({ layout.content.left, label_y, 0.0f }
						, font_size
						, counter.label
						, panel.font
						, panel.material
						, COLOR4_WHITE
						);
				}
				if (has_value) {
					_gui->text({ value_x, value_y, 0.0f }
						, font_size
						, row
						, panel.font
						, panel.material
						, COLOR4_WHITE
						);
				}
			}
		}
	}

	void reload(const StatConfigResource *config)
	{
		using namespace stat_internal;

		_config = config;
	}

} // namespace stat_globals

} // namespace crown
