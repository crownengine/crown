/*
 * Copyright (c) 2012-2026 Daniele Bartolini and individual contributors.
 * License: https://github.com/dbartolini/crown/blob/master/LICENSE
 */

#include "device/graph.h"

#if CROWN_DEBUG

#include "core/json/sjson.h"
#include "core/list.inl"
#include "core/math/color4.inl"
#include "core/math/constants.h"
#include "core/math/matrix4x4.inl"
#include "core/memory/temp_allocator.inl"
#include "core/profiler.h"
#include "core/strings/dynamic_string.inl"
#include "core/strings/string_id.inl"
#include "device/console_server.h"
#include "device/device.h"
#include "device/log.h"
#include "world/debug_line.h"
#include <errno.h>
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

struct ColorInfo
{
	StringId32 name;
	Color4 color;
};

static const ColorInfo s_colors[] =
{
	{ STRING_ID_32("red", UINT32_C(0x493d87b1)),    COLOR4_RED    },
	{ STRING_ID_32("green", UINT32_C(0xaba60fc3)),  COLOR4_GREEN  },
	{ STRING_ID_32("blue", UINT32_C(0x399a4f18)),   COLOR4_BLUE   },
	{ STRING_ID_32("yellow", UINT32_C(0x4ec858fe)), COLOR4_YELLOW }
};

static u32 name_to_color_index(StringId32 name)
{
	for (u32 i = 0; i < countof(s_colors); ++i) {
		if (s_colors[i].name == name)
			return i;
	}

	return UINT32_MAX;
}

/// Plots graphs for debugging purposes.
///
/// @ingroup Device
struct Graph
{
#define GRAPH_MAX_AXES 3

	struct ChannelData
	{
		enum { MAX_SAMPLES = 1024 };

		ProfilerEventType::Enum type;
		StringId32 field;
		u32 size;
		u32 head;
		Vector3 *samples;
		Color4 color;
	};

	Allocator *_allocator;
	f32 _range_min;
	f32 _range_max;
	u32 _num_samples;
	DynamicString _name;
	ListNode _node;
	bool _visible : 1;
	bool _range_auto : 1;
	bool _range_dirty : 1; // Whether the range needs to be recomputed.
	enum Layout { FILL, LEFT, RIGHT, BOTTOM, TOP } _layout;
	Array<ChannelData> _channels;

	Graph(Allocator &a)
		: _allocator(&a)
		, _range_min(0.0f)
		, _range_max(0.0f)
		, _num_samples(ChannelData::MAX_SAMPLES)
		, _name(a)
		, _visible(true)
		, _range_auto(true)
		, _range_dirty(true)
		, _layout(FILL)
		, _channels(a)
	{
		_node.next = NULL;
		_node.prev = NULL;
	}

	void set_range(f32 range_min, f32 range_max, bool range_auto)
	{
		if (range_auto) {
			_range_dirty = _range_auto != range_auto;
		} else {
			_range_min = range_min;
			_range_max = range_max;
		}

		_range_auto = range_auto;
	}

	void set_samples(u32 num_samples)
	{
		_num_samples = min(u32(ChannelData::MAX_SAMPLES), num_samples);
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
				for (u32 i = 0; i < array::size(_channels); ++i) {
					if (_channels[i].field == StringId32(rf->name)) {
						sample(i, rf->value);
						break;
					}
				}
			}
				cur += size;
				break;

			case ProfilerEventType::RECORD_VECTOR3: {
				RecordVector3 *rv = (RecordVector3 *)cur;
				for (u32 i = 0; i < array::size(_channels); ++i) {
					if (_channels[i].field == StringId32(rv->name)) {
						sample(i, rv->value);
						break;
					}
				}
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

	void add(StringId32 field)
	{
		ChannelData cd;
		cd.field = field;
		cd.size = 0;
		cd.head = 0;
		cd.samples = (Vector3 *)_allocator->allocate(sizeof(Vector3) * ChannelData::MAX_SAMPLES);
		cd.color = COLOR4_YELLOW;

		array::push_back(_channels, cd);
	}

	void add(const char *field)
	{
		add(StringId32(field));
	}

	void remove(StringId32 field)
	{
		for (u32 i = 0; i < array::size(_channels); ++i) {
			if (_channels[i].field == field) {
				_allocator->deallocate(_channels[i].samples);
				u32 last_index = array::size(_channels) - 1;
				if (i != last_index)
					_channels[i] = _channels[last_index];
				array::pop_back(_channels);
				return;
			}
		}
	}

	void remove(const char *field)
	{
		remove(StringId32(field));
	}

	void sample(u32 samples_index, f32 value)
	{
		ChannelData &cd = _channels[samples_index];
		cd.type = ProfilerEventType::RECORD_FLOAT;
		cd.samples[cd.head] = { value, 0.0f, 0.0f };
		cd.head = (cd.head + 1) % ChannelData::MAX_SAMPLES;
		cd.size = min(cd.size + 1, (u32)ChannelData::MAX_SAMPLES);

		if (_range_auto) {
			if (CE_UNLIKELY(_range_dirty)) {
				for (u32 ii = 0; ii < array::size(_channels); ++ii) {
					ChannelData &cd = _channels[ii];
					for (u32 jj = 0; jj < cd.size; ++jj) {
						const f32 val = cd.samples[(cd.head - 1 - jj) % ChannelData::MAX_SAMPLES].x;

						if (jj == 0) {
							_range_min = val;
							_range_max = val;
						} else {
							_range_min = min(_range_min, val);
							_range_max = max(_range_max, val);
						}
					}
				}
				_range_dirty = false;
			} else {
				_range_min = min(_range_min, value);
				_range_max = max(_range_max, value);
			}
		}
	}

	void sample(u32 samples_index, const Vector3 &value)
	{
		ChannelData &cd = _channels[samples_index];
		cd.type = ProfilerEventType::RECORD_VECTOR3;
		cd.samples[cd.head] = value;
		cd.head = (cd.head + 1) % ChannelData::MAX_SAMPLES;
		cd.size = min(cd.size + 1, (u32)ChannelData::MAX_SAMPLES);

		if (_range_auto) {
			if (CE_UNLIKELY(_range_dirty)) {
				for (u32 ii = 0; ii < array::size(_channels); ++ii) {
					ChannelData &cd = _channels[ii];
					for (u32 jj = 0; jj < cd.size; ++jj) {
						const Vector3 val = cd.samples[(cd.head - 1 - jj) % ChannelData::MAX_SAMPLES];

						if (jj == 0) {
							_range_min = min(val.x, val.y, val.z);
							_range_max = max(val.x, val.y, val.z);
						} else {
							_range_min = min(_range_min, min(value.x, value.y, value.z));
							_range_max = max(_range_max, max(value.x, value.y, value.z));
						}
					}
				}
				_range_dirty = false;
			} else {
				_range_min = min(_range_min, min(value.x, value.y, value.z));
				_range_max = max(_range_max, max(value.x, value.y, value.z));
			}
		}
	}

	void draw(DebugLine &dl, u16 window_width, u16 window_height)
	{
		if (!_visible)
			return;

		sample_with_filter(profiler_globals::buffer_begin(), profiler_globals::buffer_end());

		auto remap = [](f32 x, f32 in_min, f32 in_max, f32 out_min, f32 out_max) -> f32 {
			return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
		};

		const f32 margin_padding = 32.0; // Pixels of padding inside window margins.
		const f32 window_width_padded  = f32(window_width) - margin_padding;
		const f32 window_height_padded = f32(window_height) - margin_padding;

		// Margins in window-coordinates.
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
			x_step = f32(margin_right - margin_left) / f32(_num_samples - 1) / 2.0f;
			y_min = margin_bottom;
			y_max = margin_top;
			break;

		case RIGHT:
			x_start = 0.0f;
			x_step = f32(margin_right - margin_left) / f32(_num_samples - 1) / 2.0f;
			y_min = margin_bottom;
			y_max = margin_top;
			break;

		case BOTTOM:
			x_start = margin_left;
			x_step = f32(margin_right - margin_left) / f32(_num_samples - 1);
			y_min = margin_bottom;
			y_max = 0.0f;
			break;

		case TOP:
			x_start = margin_left;
			x_step = f32(margin_right - margin_left) / f32(_num_samples - 1);
			y_min = 0.0f;
			y_max = margin_top;
			break;

		case FILL:
		default:
			x_start = margin_left;
			x_step = f32(margin_right - margin_left) / f32(_num_samples - 1);
			y_min = margin_bottom;
			y_max = margin_top;
			break;
		}

		f32 x_end = x_start + x_step*(_num_samples - 1);

		// Draw margin top.
		dl.add_line({ x_start, y_max, 0.0f }
			, { x_end, y_max, 0.0f }
			, COLOR4_ORANGE
			);
		// Draw margin right.
		dl.add_line({ x_end, y_max, 0.0f }
			, { x_end, y_min, 0.0f }
			, COLOR4_ORANGE
			);
		// Draw margin bottom.
		dl.add_line({ x_end, y_min, 0.0f }
			, { x_start, y_min, 0.0f }
			, COLOR4_ORANGE
			);
		// Draw margin left.
		dl.add_line({ x_start, y_min, 0.0f }
			, { x_start, y_max, 0.0f }
			, COLOR4_ORANGE
			);

		// For each channel.
		for (u32 cc = 0; cc < array::size(_channels); ++cc) {
			ChannelData &cd = _channels[cc];
			u32 cur_sample = (cd.head - 1 - _num_samples) % ChannelData::MAX_SAMPLES;

			// For each sample.
			for (u32 ii = 0; ii < (_num_samples - 1); ++ii) {
				// Do not draw invalid samples.
				if (cd.size >= _num_samples || ii >= _num_samples - cd.size) {
					const u32 num_axis = cd.type == ProfilerEventType::RECORD_FLOAT ? 1 : 3;
					for (u32 axis = 0; axis < num_axis; ++axis) {
						f32 *a_data = to_float_ptr(cd.samples[(cur_sample + 0) % ChannelData::MAX_SAMPLES]);
						f32 *b_data = to_float_ptr(cd.samples[(cur_sample + 1) % ChannelData::MAX_SAMPLES]);
						Vector3 a;
						Vector3 b;

						a.x = x_start + (ii + 0)*x_step;
						a.y = remap(a_data[axis], _range_min, _range_max, y_min, y_max);
						a.z = 0.0f;

						b.x = x_start + (ii + 1)*x_step;
						b.y = remap(b_data[axis], _range_min, _range_max, y_min, y_max);
						b.z = 0.0f;

						dl.add_line(a, b, num_axis == 1 ? cd.color : s_colors[axis].color);
					}
				}

				cur_sample = (cur_sample + 1) % ChannelData::MAX_SAMPLES;
			}
		}

		// Draw labels.
#define TEXT_PADDING 6.0f
		float str_x = x_start + TEXT_PADDING;
		char buf[64] = {};
		stbsp_snprintf(buf, sizeof(buf), "%g", _range_min);
		draw_string(dl, buf, str_x, y_min + TEXT_PADDING);
		stbsp_snprintf(buf, sizeof(buf), "%g", _range_max);
		draw_string(dl, buf, str_x, y_max - TEXT_PADDING - 32.0f);

		dl.submit(View::GRAPH);
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
		if (subcmd == "help") {
			logi(GRAPH, "make       Create a new graph.");
			logi(GRAPH, "list       List graphs.");
			logi(GRAPH, "range      Set the range of a graph.");
			logi(GRAPH, "add        Add a field to a graph.");
			logi(GRAPH, "remove     Remove a field from a graph.");
			logi(GRAPH, "hide       Hide a graph.");
			logi(GRAPH, "show       Show a graph.");
			logi(GRAPH, "layout     Set the layout of a graph.");
			logi(GRAPH, "color      Set the color of a field in a graph.");
			logi(GRAPH, "samples    Set the number of samples to show in a graph.");
		} else if (subcmd == "make") {
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
				cs.error(client_id, "Usage: graph range <graph> [min max]");
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
				cs.error(client_id, "Usage: graph add <graph> <field>");
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
		} else if (subcmd == "remove") {
			if (array::size(args) != 4) {
				cs.error(client_id, "Usage: graph remove <graph> <field>");
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
			graph->remove(field.c_str());
		} else if (subcmd == "hide") {
			if (array::size(args) != 3) {
				cs.error(client_id, "Usage: graph hide <graph>");
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
				cs.error(client_id, "Usage: graph show <graph>");
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
				cs.error(client_id, "Usage: graph layout <graph> <type>");
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
		} else if (subcmd == "color") {
			if (array::size(args) != 5) {
				cs.error(client_id, "Usage: graph color <graph> <field> <color>");
				return;
			}

			DynamicString name(ta);
			DynamicString color_name(ta);
			StringId32 field = sjson::parse_string_id(args[3]);
			sjson::parse_string(name, args[2]);
			sjson::parse_string(color_name, args[4]);

			Color4 color;
			u32 color_index = name_to_color_index(color_name.to_string_id());
			if (color_index == UINT32_MAX) {
				if (color_name.length() != 3 && color_name.length() != 6) {
					cs.error(client_id, "Invalid color");
					return;
				} else {
					// Decode hex color.
					errno = 0;
					u32 val = strtol(color_name.c_str(), NULL, 16);
					if (errno != ERANGE && errno != EINVAL) {
						if (color_name.length() == 3) {
							u8 r = ((val & 0xf00) >> 8) | ((val & 0xf00) >> 4);
							u8 g = ((val & 0x0f0) >> 4) | ((val & 0x0f0) >> 0);
							u8 b = ((val & 0x00f) >> 0) | ((val & 0x00f) << 4);
							color = from_rgb(r, g, b);
						} else {
							color = from_rgb(val);
						}
					}
				}
			} else {
				color = s_colors[color_index].color;
			}

			Graph *graph = graph::find(_graphs, name.c_str());
			if (graph == NULL) {
				cs.error(client_id, "Graph not found");
				return;
			}

			for (u32 i = 0; i < array::size(graph->_channels); ++i) {
				if (graph->_channels[i].field == field) {
					graph->_channels[i].color = color;
					break;
				}
			}
		} else if (subcmd == "samples") {
			if (array::size(args) != 4) {
				cs.error(client_id, "Usage: graph samples <graph> <samples>");
				return;
			}

			DynamicString name(ta);
			DynamicString samples(ta);
			sjson::parse_string(name, args[2]);
			sjson::parse_string(samples, args[3]);

			Graph *graph = graph::find(_graphs, name.c_str());
			if (graph == NULL) {
				cs.error(client_id, "Graph not found");
				return;
			}

			graph->set_samples((u32)sjson::parse_int(samples.c_str()));
		} else {
			cs.error(client_id, "Unknown graph parameter");
		}
	}

} // namespace graph_internal

} // namespace crown

#endif // CROWN_DEBUG

namespace crown
{
namespace graph_globals
{
	Allocator *_allocator = NULL;
	DebugLine *_lines = NULL;

	void init(Allocator &a, Pipeline &pl, ConsoleServer &cs)
	{
#if CROWN_DEBUG
		_allocator = &a;
		_lines = debug_line::create(a, pl, false);

		cs.register_command_name("graph", "Plot selected profiler data.", graph_internal::handle_command, NULL);
#else
		CE_UNUSED_3(a, pl, cs);
		CE_NOOP();
#endif
	}

	void shutdown()
	{
#if CROWN_DEBUG
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
#else
		CE_NOOP();
#endif
	}

	void draw_all(u16 window_width, u16 window_height)
	{
#if CROWN_DEBUG
		_lines->reset();

		ListNode *cur;
		list_for_each(cur, &_graphs)
		{
			Graph *graph = (Graph *)container_of(cur, Graph, _node);
			graph->draw(*_lines, window_width, window_height);
		}
#else
		CE_UNUSED_2(window_width, window_height);
		CE_NOOP();
#endif
	}

} // namespace graph_globals

} // namespace crown
