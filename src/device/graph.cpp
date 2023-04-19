/*
 * Copyright (c) 2012-2023 Daniele Bartolini and individual contributors.
 * License: https://github.com/dbartolini/crown/blob/master/LICENSE
 */

#include "device/graph.h"

#if CROWN_DEBUG

#include "core/json/sjson.h"
#include "core/list.inl"
#include "core/math/constants.h"
#include "core/math/matrix4x4.inl"
#include "core/memory/temp_allocator.inl"
#include "core/strings/dynamic_string.inl"
#include "core/strings/string_id.inl"
#include "device/console_server.h"
#include "device/device.h"
#include "device/log.h"
#include "device/profiler.h"
#include "world/debug_line.h"
#include <stb_sprintf.h>

LOG_SYSTEM(GRAPH, "graph")

namespace crown
{
static ListNode _graphs = LIST_INIT_HEAD(_graphs);

static const Vector3 segments[8][2] =
{
#define SEGMENTS_ASPECT_RATIO (1.0f/1.618f)
	//
	// E F F A
	// E     A
	// E     A
	// E G G A
	// D     B
	// D     B
	// D C C B
	//
	{ { 1.0f*SEGMENTS_ASPECT_RATIO, 1.0f, 0.0f }, {  1.0f*SEGMENTS_ASPECT_RATIO,  0.5f, 0.0f } }, // A
	{ { 1.0f*SEGMENTS_ASPECT_RATIO, 0.5f, 0.0f }, {  1.0f*SEGMENTS_ASPECT_RATIO,  0.0f, 0.0f } }, // B
	{ { 0.0f*SEGMENTS_ASPECT_RATIO, 0.0f, 0.0f }, {  1.0f*SEGMENTS_ASPECT_RATIO,  0.0f, 0.0f } }, // C
	{ { 0.0f*SEGMENTS_ASPECT_RATIO, 0.0f, 0.0f }, {  0.0f*SEGMENTS_ASPECT_RATIO,  0.5f, 0.0f } }, // D
	{ { 0.0f*SEGMENTS_ASPECT_RATIO, 0.5f, 0.0f }, {  0.0f*SEGMENTS_ASPECT_RATIO,  1.0f, 0.0f } }, // E
	{ { 0.0f*SEGMENTS_ASPECT_RATIO, 1.0f, 0.0f }, {  1.0f*SEGMENTS_ASPECT_RATIO,  1.0f, 0.0f } }, // F
	{ { 0.0f*SEGMENTS_ASPECT_RATIO, 0.5f, 0.0f }, {  1.0f*SEGMENTS_ASPECT_RATIO,  0.5f, 0.0f } }, // G
	{ { 0.0f*SEGMENTS_ASPECT_RATIO, 0.0f, 0.0f }, { -0.1f*SEGMENTS_ASPECT_RATIO, -0.4f, 0.0f } }, // H
};

static void draw_string(DebugLine &dl, const char *str, float x, float y)
{
	u8 table[256] = {0};
	//             HGFEDCBA
	table[' '] = 0b00000000;
	table['0'] = 0b00111111;
	table['1'] = 0b00000011;
	table['2'] = 0b01101101;
	table['3'] = 0b01100111;
	table['4'] = 0b01010011;
	table['5'] = 0b01110110;
	table['6'] = 0b01111110;
	table['7'] = 0b00100011;
	table['8'] = 0b01111111;
	table['9'] = 0b01110111;
	table['.'] = 0b10000000;
	table['-'] = 0b01000000;
	table['a'] = 0b01111011;
	table['b'] = 0b01011110;
	table['c'] = 0b00111100;
	table['d'] = 0b01001111;
	table['e'] = 0b01111100;
	table['f'] = 0b01111000;
	table['A'] = table['a'];
	table['B'] = table['b'];
	table['C'] = table['c'];
	table['D'] = table['d'];
	table['E'] = table['e'];
	table['F'] = table['f'];

	Vector3 advance;
	advance.x = x;
	advance.y = y;
	advance.z = 0.0f;
	for (const char *ch = str; *ch; ++ch) {
		for (int i = 0; i < 8; ++i) {
			if ((table[(int)*ch] & (1 << i)) != 0)
				dl.add_line((32.0f * segments[i][0]) + advance, (32.0f * segments[i][1]) + advance, COLOR4_YELLOW);
		}
		float x_adv = 32.0f*SEGMENTS_ASPECT_RATIO;
		advance.x += *ch != '.' ? x_adv*1.2f : x_adv*0.2f;
	}
}

/// Plots graphs for debugging purposes.
///
/// @ingroup Device
struct Graph
{
#define GRAPH_MAX_AXES 3
#define GRAPH_MAX_SAMPLES 128
	s32 _head;
	f32 _samples[GRAPH_MAX_AXES][GRAPH_MAX_SAMPLES];
	f32 _range_min;
	f32 _range_max;
	DynamicString _name;
	ListNode _node;
	char _field[32];
	bool _visible;
	bool _range_auto;
	enum Layout { FILL, LEFT, RIGHT, BOTTOM, TOP } _layout;

	Graph(Allocator &a)
		: _head(0)
		, _range_min(0.0f)
		, _range_max(0.0f)
		, _name(a)
		, _visible(true)
		, _range_auto(true)
		, _layout(FILL)
	{
		_node.next = NULL;
		_node.prev = NULL;

		memset(_samples, 0, sizeof(_samples));
		memset(_field, 0, sizeof(_field));
	}

	void set_range(f32 range_min, f32 range_max, bool range_auto)
	{
		if (range_auto) {
			_range_min = 0.0f;
			_range_max = 0.0f;
		} else {
			_range_min = range_min;
			_range_max = range_max;
		}

		_range_auto = range_auto;
	}

	void sample_with_filter(const char *cur, const char *end)
	{
		while (cur != end) {
			const u32 type = *(u32 *)cur;
			cur += sizeof(u32);
			if (type == ProfilerEventType::COUNT)
				break;

			const u32 size = *(u32 *)cur;
			cur += sizeof(u32);

			switch (type) {
			case ProfilerEventType::RECORD_FLOAT: {
				RecordFloat *rf = (RecordFloat *)cur;
				if (strcmp(_field, rf->name) == 0)
					sample(rf->value);
			}
				cur += size;
				break;

			case ProfilerEventType::RECORD_VECTOR3: {
				RecordVector3 *rv = (RecordVector3 *)cur;
				if (strcmp(_field, rv->name) == 0)
					sample(rv->value);
			}
				cur += size;
				break;

			case ProfilerEventType::ENTER_PROFILE_SCOPE:
			case ProfilerEventType::LEAVE_PROFILE_SCOPE:
			case ProfilerEventType::ALLOCATE_MEMORY:
			case ProfilerEventType::DEALLOCATE_MEMORY:
				cur += size;
				break;

			default:
				CE_FATAL("Unknown profiler event type");
				break;
			}
		}
	}

	void add(const char *field)
	{
		strcpy(_field, field);
	}

	void sample(f32 value)
	{
		if (_range_auto) {
			if (_range_min > value)
				_range_min = value;
			if (_range_max < value)
				_range_max = value;
		}
		_samples[0][_head] = value;
		_head = (_head + 1) % GRAPH_MAX_SAMPLES;
	}

	void sample(const Vector3 &value)
	{
		if (_range_auto) {
			if (_range_min > value.x)
				_range_min = value.x;
			if (_range_min > value.y)
				_range_min = value.y;
			if (_range_min > value.z)
				_range_min = value.z;
			if (_range_max < value.x)
				_range_max = value.x;
			if (_range_max < value.y)
				_range_max = value.y;
			if (_range_max < value.z)
				_range_max = value.z;
		}
		_samples[0][_head] = value.x;
		_samples[1][_head] = value.y;
		_samples[2][_head] = value.z;
		_head = (_head + 1) % GRAPH_MAX_SAMPLES;
	}

	void draw(DebugLine &dl, u16 window_width, u16 window_height)
	{
		if (!_visible)
			return;

		sample_with_filter(profiler_globals::buffer_begin(), profiler_globals::buffer_end());

		auto remap = [](f32 x, f32 in_min, f32 in_max, f32 out_min, f32 out_max) -> f32 {
			return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
		};

		Color4 colors[] =
		{
			COLOR4_YELLOW,
			COLOR4_RED,
			COLOR4_GREEN
		};
		CE_STATIC_ASSERT(countof(colors) == GRAPH_MAX_AXES);

		const f32 margin_padding = 32.0; // Pixels of padding inside window margins
		const f32 window_width_padded  = f32(window_width) - margin_padding;
		const f32 window_height_padded = f32(window_height) - margin_padding;

		// Margins in window-coordinates
		const f32 margin_right   =  window_width_padded / 2.0f;
		const f32 margin_left    = -margin_right;
		const f32 margin_top     =  window_height_padded / 2.0f;
		const f32 margin_bottom  = -margin_top;

		f32 x_start;
		f32 x_step;
		f32 y_min;
		f32 y_max;
		switch (_layout) {
		case LEFT:
			x_start = margin_left;
			x_step = f32(margin_right - margin_left) / f32(GRAPH_MAX_SAMPLES - 1) / 2.0f;
			y_min = margin_bottom;
			y_max = margin_top;
			break;

		case RIGHT:
			x_start = 0.0f;
			x_step = f32(margin_right - margin_left) / f32(GRAPH_MAX_SAMPLES - 1) / 2.0f;
			y_min = margin_bottom;
			y_max = margin_top;
			break;

		case BOTTOM:
			x_start = margin_left;
			x_step = f32(margin_right - margin_left) / f32(GRAPH_MAX_SAMPLES - 1);
			y_min = margin_bottom;
			y_max = 0.0f;
			break;

		case TOP:
			x_start = margin_left;
			x_step = f32(margin_right - margin_left) / f32(GRAPH_MAX_SAMPLES - 1);
			y_min = 0.0f;
			y_max = margin_top;
			break;

		case FILL:
		default:
			x_start = margin_left;
			x_step = f32(margin_right - margin_left) / f32(GRAPH_MAX_SAMPLES - 1);
			y_min = margin_bottom;
			y_max = margin_top;
			break;
		}

		// Draw margin top
		dl.add_line(vector3(x_start, y_max, 0.0f)
			, vector3(x_start + x_step*(GRAPH_MAX_SAMPLES - 1), y_max, 0.0f)
			, COLOR4_ORANGE
			);
		// Draw margin right
		dl.add_line(vector3(x_start + x_step*(GRAPH_MAX_SAMPLES - 1), y_max, 0.0f)
			, vector3(x_start + x_step*(GRAPH_MAX_SAMPLES - 1), y_min, 0.0f)
			, COLOR4_ORANGE
			);
		// Draw margin bottom
		dl.add_line(vector3(x_start + x_step*(GRAPH_MAX_SAMPLES - 1), y_min, 0.0f)
			, vector3(x_start, y_min, 0.0f)
			, COLOR4_ORANGE
			);
		// Draw margin left
		dl.add_line(vector3(x_start, y_min, 0.0f)
			, vector3(x_start, y_max, 0.0f)
			, COLOR4_ORANGE
			);

		// For each channel
		for (s32 cc = 0; cc < GRAPH_MAX_AXES; ++cc) {
			// For each sample in the channel
			for (s32 ii = 0, oldest = _head; ii < GRAPH_MAX_SAMPLES - 1; ++ii) {
				f32 value[2];
				value[0] = remap(_samples[cc][(oldest + 0) % GRAPH_MAX_SAMPLES], _range_min, _range_max, y_min, y_max);
				value[1] = remap(_samples[cc][(oldest + 1) % GRAPH_MAX_SAMPLES], _range_min, _range_max, y_min, y_max);
				dl.add_line(vector3(x_start + (ii + 0)*x_step, value[0], 0.0f)
					, vector3(x_start + (ii + 1)*x_step, value[1], 0.0f)
					, colors[cc]
					);
				oldest = (oldest + 1) % GRAPH_MAX_SAMPLES;
			}
		}

		// Draw labels
#define TEXT_PADDING 6.0f
		float str_x = x_start + TEXT_PADDING;
		char buf[64] = {};
		stbsp_snprintf(buf, sizeof(buf), "%g", _range_min);
		draw_string(dl, buf, str_x, y_min + TEXT_PADDING);
		stbsp_snprintf(buf, sizeof(buf), "%g", _range_max);
		draw_string(dl, buf, str_x, y_max - TEXT_PADDING - 32.0f);

		dl.submit(VIEW_GRAPH);
	}
};

namespace graph
{
	/// Creates a new graph with the given @a name and appends it
	/// to the list @a head.
	Graph *create(ListNode &head, const char *name)
	{
		Graph *graph = CE_NEW(default_allocator(), Graph)(default_allocator());
		graph->_name = name;
		list::add(graph->_node, head);
		return graph;
	}

	/// Destroys the given @a graph.
	void destroy(Graph *graph)
	{
		list::remove(graph->_node);
		CE_DELETE(default_allocator(), graph);
	}

	/// Returns the graph with the given @a name or NULL if no graph is found.
	Graph *find(ListNode &head, const char *name)
	{
		ListNode *cur;
		list_for_each(cur, &head)
		{
			Graph *graph = (Graph *)container_of(cur, Graph, _node);
			if (graph->_name == name)
				return graph;
		}

		return NULL;
	}

} // namespace graph

namespace graph_internal
{
	void handle_command(ConsoleServer &cs, u32 client_id, const JsonArray &args, void * /*user_data*/)
	{
		TempAllocator1024 ta;

		if (array::size(args) < 2) {
			cs.error(client_id, "Usage: graph make <name>");
			return;
		}

		DynamicString subcmd(ta);
		sjson::parse_string(subcmd, args[1]);
		if (subcmd == "make") {
			if (array::size(args) != 3) {
				cs.error(client_id, "Usage: graph make <name>");
				return;
			}

			DynamicString name(ta);
			sjson::parse_string(name, args[2]);

			Graph *graph = graph::find(_graphs, name.c_str());
			if (graph != NULL) {
				cs.error(client_id, "Graph with this name already exists");
				return;
			}
			graph::create(_graphs, name.c_str());
		} else if (subcmd == "list") {
			ListNode *cur;
			list_for_each(cur, &_graphs)
			{
				Graph *graph = (Graph *)container_of(cur, Graph, _node);
				logi(GRAPH, "%s", graph->_name.c_str());
			}
		} else if (subcmd == "range") {
			if (array::size(args) != 3 && array::size(args) != 5) {
				cs.error(client_id, "Usage: graph range <name> [min max]");
				return;
			}

			DynamicString name(ta);
			DynamicString min(ta);
			DynamicString max(ta);
			sjson::parse_string(name, args[2]);
			if (array::size(args) == 5) {
				sjson::parse_string(min, args[3]);
				sjson::parse_string(max, args[4]);
			} else {
				min = "0";
				max = "0";
			}

			Graph *graph = graph::find(_graphs, name.c_str());
			if (graph == NULL) {
				cs.error(client_id, "Graph not found");
				return;
			}
			graph->set_range(sjson::parse_float(min.c_str()), sjson::parse_float(max.c_str()), array::size(args) == 3);
		} else if (subcmd == "add") {
			if (array::size(args) != 4) {
				cs.error(client_id, "Usage: graph add <name> <field>");
				return;
			}

			DynamicString name(ta);
			DynamicString field(ta);
			sjson::parse_string(name, args[2]);
			sjson::parse_string(field, args[3]);

			Graph *graph = graph::find(_graphs, name.c_str());
			if (graph == NULL) {
				cs.error(client_id, "Graph not found");
				return;
			}
			graph->add(field.c_str());
		} else if (subcmd == "hide") {
			if (array::size(args) != 3) {
				cs.error(client_id, "Usage: graph hide <name>");
				return;
			}

			DynamicString name(ta);
			sjson::parse_string(name, args[2]);

			Graph *graph = graph::find(_graphs, name.c_str());
			if (graph == NULL) {
				cs.error(client_id, "Graph not found");
				return;
			}
			graph->_visible = false;
		} else if (subcmd == "show") {
			if (array::size(args) != 3) {
				cs.error(client_id, "Usage: graph show <name>");
				return;
			}

			DynamicString name(ta);
			sjson::parse_string(name, args[2]);

			Graph *graph = graph::find(_graphs, name.c_str());
			if (graph == NULL) {
				cs.error(client_id, "Graph not found");
				return;
			}
			graph->_visible = true;
		} else if (subcmd == "layout") {
			if (array::size(args) != 4) {
				cs.error(client_id, "Usage: graph layout <name> <type>");
				return;
			}

			DynamicString name(ta);
			DynamicString type(ta);
			sjson::parse_string(name, args[2]);
			sjson::parse_string(type, args[3]);

			s32 lt = Graph::FILL;
			if (type == "fill") {
				lt = Graph::FILL;
			} else if (type == "left") {
				lt = Graph::LEFT;
			} else if (type == "right") {
				lt = Graph::RIGHT;
			} else if (type == "bottom") {
				lt = Graph::BOTTOM;
			} else if (type == "top") {
				lt = Graph::TOP;
			} else {
				cs.error(client_id, "Invalid layout type");
				return;
			}

			Graph *graph = graph::find(_graphs, name.c_str());
			if (graph == NULL) {
				cs.error(client_id, "Graph not found");
				return;
			}
			graph->_layout = (Graph::Layout)lt;
		} else {
			cs.error(client_id, "Unknown graph command");
		}
	}

} // namespace graph_internal

namespace graph_globals
{
	Allocator *_allocator = NULL;
	DebugLine *_lines = NULL;

	void init(Allocator &a, ShaderManager &sm, ConsoleServer &cs)
	{
		_allocator = &a;
		_lines = CE_NEW(a, DebugLine)(sm, false);

		cs.register_command_name("graph", "Plot selected profiler data", graph_internal::handle_command, NULL);
	}

	void shutdown()
	{
		// Destroy all graphs
		ListNode *cur;
		ListNode *tmp;
		list_for_each_safe(cur, tmp, &_graphs)
		{
			Graph *graph = (Graph *)container_of(cur, Graph, _node);
			CE_DELETE(default_allocator(), graph);
		}

		CE_DELETE(*_allocator, _lines);
		_lines = NULL;
		_allocator = NULL;
	}

	void draw_all(u16 window_width, u16 window_height)
	{
		_lines->reset();

		ListNode *cur;
		list_for_each(cur, &_graphs)
		{
			Graph *graph = (Graph *)container_of(cur, Graph, _node);
			graph->draw(*_lines, window_width, window_height);
		}
	}

} // namespace graph_globals

} // namespace crown

#endif // CROWN_DEBUG
