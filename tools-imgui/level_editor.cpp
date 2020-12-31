/*
 * Copyright (c) 2012-2021 Daniele Bartolini et al.
 * License: https://github.com/dbartolini/crown/blob/master/LICENSE
 */

#if CROWN_TOOLS

#include "core/containers/vector.inl"
#include "core/filesystem/file.h"
#include "core/filesystem/filesystem_disk.h"
#include "core/filesystem/path.h"
#include "core/json/json.h"
#include "core/json/json_object.inl"
#include "core/json/sjson.h"
#include "core/math/vector2.inl"
#include "core/memory/temp_allocator.inl"
#include "core/network/ip_address.h"
#include "core/network/socket.h"
#include "core/process.h"
#include "core/strings/dynamic_string.inl"
#include "core/strings/string_id.inl"
#include "core/strings/string_stream.inl"
#include "device/device.h"
#include "device/device_options.h"
#include "device/input_device.h"
#include "device/input_manager.h"
#include "device/input_types.h"
#include "device/log.h"
#include "device/pipeline.h"
#include "device/types.h"
#include "imgui_context.h"
#include "resource/resource_manager.h"
#include "resource/texture_resource.h"
#include "tool_api.h"
#include <iconfontheaders/icons_material_design.h>
#include <imgui.h>
#include <nfd.h>
#include <time.h>
#if CROWN_PLATFORM_POSIX
	#include <sys/time.h>
#endif

LOG_SYSTEM(LEVEL_EDITOR, "level_editor")

namespace crown
{
struct Project
{
	DynamicString _data_dir;

	Project(Allocator& a)
		: _data_dir(a)
	{
	}
};

static u16 _width = 1280;
static u16 _height = 720;

static struct LevelEditor* _editor;
static struct Project* _project;
static TCPSocket _client;
static Process _game_process;

struct StartGame
{
	enum Enum
	{
		NORMAL,
		TEST
	};
};

static void stop_game()
{
	if (_game_process.spawned())
	{
		_game_process.force_exit();
		_game_process.wait();
	}
}

// FIXME: duplicated from compile_options.h
#if CROWN_PLATFORM_LINUX
	#define EXE_PREFIX "./"
	#define EXE_SUFFIX ""
#elif CROWN_PLATFORM_WINDOWS
	#define EXE_PREFIX ""
	#define EXE_SUFFIX ".exe"
#else
	#error "Unknown platform"
#endif // CROWN_PLATFORM_LINUX

#define EXE_PATH(exe) EXE_PREFIX exe EXE_SUFFIX

static void start_game(StartGame::Enum sg, const char* data_dir)
{
	// Stop any previously launched game
	stop_game();

	const char* argv[] =
	{
#if CROWN_DEBUG && !CROWN_DEVELOPMENT
		EXE_PATH("crown-debug"),
#else
		EXE_PATH("crown-development"),
#endif
		"--data-dir", data_dir,
		"--console-port", "12345",
		// "--wait-console",
		"--lua-string", sg == StartGame::TEST ? "TEST=true" : "",
		NULL
	};
	_game_process.spawn(argv);
}

struct Pivot
{
	enum Enum
	{
		TOP_LEFT,
		TOP_CENTER,
		TOP_RIGHT,
		LEFT,
		CENTER,
		RIGHT,
		BOTTOM_LEFT,
		BOTTOM_CENTER,
		BOTTOM_RIGHT,
		COUNT
	};
};

static const char* pivot_names[] =
{
	"Top Left",      // Pivot::TOP_LEFT
	"Top Right",     // Pivot::TOP_CENTER
	"Top Center",    // Pivot::TOP_RIGHT
	"Left",          // Pivot::LEFT
	"Center",        // Pivot::CENTER
	"Right",         // Pivot::RIGHT
	"Bottom Left",   // Pivot::BOTTOM_LEFT
	"Bottom Center", // Pivot::BOTTOM_CENTER
	"Bottom Right"   // Pivot::BOTTOM_RIGHT
};
CE_STATIC_ASSERT(countof(pivot_names) == Pivot::COUNT);

Vector2 sprite_cell_xy(int r, int c, int offset_x, int offset_y, int cell_w, int cell_h, int spacing_x, int spacing_y)
{
	int x0 = offset_x + c*cell_w + c*spacing_x;
	int y0 = offset_y + r*cell_h + r*spacing_y;
	return vector2((f32)x0, (f32)y0);
}

Vector2 sprite_cell_pivot_xy(int cell_w, int cell_h, int pivot)
{
	int pivot_x = 0;
	int pivot_y = 0;

	switch (pivot)
	{
	case Pivot::TOP_LEFT:
		pivot_x = 0;
		pivot_y = 0;
		break;

	case Pivot::TOP_CENTER:
		pivot_x = cell_w / 2;
		pivot_y = 0;
		break;

	case Pivot::TOP_RIGHT:
		pivot_x = cell_w;
		pivot_y = 0;
		break;

	case Pivot::BOTTOM_LEFT:
		pivot_x = 0;
		pivot_y = cell_h;
		break;

	case Pivot::BOTTOM_CENTER:
		pivot_x = cell_w / 2;
		pivot_y = cell_h;
		break;

	case Pivot::BOTTOM_RIGHT:
		pivot_x = cell_w;
		pivot_y = cell_h;
		break;

	case Pivot::LEFT:
		pivot_x = 0;
		pivot_y = cell_h / 2;
		break;

	case Pivot::CENTER:
		pivot_x = cell_w / 2;
		pivot_y = cell_h / 2;
		break;

	case Pivot::RIGHT:
		pivot_x = cell_w;
		pivot_y = cell_h / 2;
		break;

	default:
		CE_FATAL("Unknown pivot");
		break;
	}

	return vector2((f32)pivot_x, (f32)pivot_y);
}

struct SpriteImporter
{
	int width;
	int height;
	int cells_h;
	int cells_v;
	bool cell_wh_auto;
	int cell_w;
	int cell_h;
	int offset_x;
	int offset_y;
	int spacing_x;
	int spacing_y;
	int pivot;
	int layer;
	int depth;

	SpriteImporter()
		: width(128)
		, height(128)
		, cells_h(4)
		, cells_v(4)
		, cell_wh_auto(false)
		, cell_w(16)
		, cell_h(16)
		, offset_x(0)
		, offset_y(0)
		, spacing_x(0)
		, spacing_y(0)
		, pivot(Pivot::CENTER)
		, layer(0)
		, depth(0)
	{
	}

	void draw()
	{
		ImGui::Columns(2);
#if 1
		ImDrawList* draw_list = ImGui::GetWindowDrawList();
		{
			// Here we are using InvisibleButton() as a convenience to 1) advance the cursor and 2) allows us to use IsItemHovered()
			// However you can draw directly and poll mouse/keyboard by yourself. You can manipulate the cursor using GetCursorPos() and SetCursorPos().
			// If you only use the ImDrawList API, you can notify the owner window of its extends by using SetCursorPos(max).
			ImVec2 canvas_pos = ImGui::GetCursorScreenPos();            // ImDrawList API uses screen coordinates!
			ImVec2 canvas_size = ImGui::GetContentRegionAvail();        // Resize canvas to what's available
			if (canvas_size.x < 50.0f) canvas_size.x = 10.0f;
			if (canvas_size.y < 50.0f) canvas_size.y = 50.0f;
			draw_list->AddRectFilledMultiColor(canvas_pos, ImVec2(canvas_pos.x + canvas_size.x, canvas_pos.y + canvas_size.y), ImColor(50,50,50), ImColor(50,50,60), ImColor(60,60,70), ImColor(50,50,60));

			// Pivot is relative to the top-left corner of the cell
			Vector2 pivot_xy = sprite_cell_pivot_xy(cell_w
				, cell_h
				, pivot
				);

			int num_v = cells_v;
			int num_h = cells_h;
			for (int h = 0; h < num_v; ++h)
			{
				for (int w = 0; w < num_h; ++w)
				{
					Vector2 cell = sprite_cell_xy(h
						, w
						, offset_x
						, offset_y
						, cell_w
						, cell_h
						, spacing_x
						, spacing_y
						);

					const int x0 = (int)cell.x;
					const int y0 = (int)cell.y;
					const int x1 = x0+(int)cell_w;
					const int y1 = y0+(int)cell_h;
					draw_list->AddRect(ImVec2(canvas_pos.x + x0, canvas_pos.y + y0)
						, ImVec2(canvas_pos.x + x1, canvas_pos.y + y1)
						, ImColor(230, 26, 26, 153)
						);

					draw_list->AddCircleFilled(ImVec2(x0 + canvas_pos.x + pivot_xy.x, y0 + canvas_pos.y + pivot_xy.y)
						, 5.0f
						, ImColor(26, 26, 230, 153)
						);
				}
			}

			ImGui::InvisibleButton("canvas", canvas_size);
			draw_list->PushClipRect(canvas_pos, ImVec2(canvas_pos.x+canvas_size.x, canvas_pos.y+canvas_size.y));      // clip lines within the canvas (if we resize it, etc.)
		}
#endif
		ImGui::NextColumn();

		ImGui::BeginGroup();
		ImGui::LabelText("Resolution", "%d x %d", width, height);

		ImGui::InputInt("Cells H", &cells_h);
		cells_h = clamp(cells_h, 1, 256);

		ImGui::InputInt("Cells V", &cells_v);
		cells_v = clamp(cells_v, 1, 256);

		ImGui::Checkbox("Cell WH auto", &cell_wh_auto);
		ImGui::InputInt("Cell W", &cell_w);
		cell_w = clamp(cell_w, 1, 4096);

		ImGui::InputInt("Cell H", &cell_h);
		cell_h = clamp(cell_h, 1, 4096);

		ImGui::InputInt("Offset X", &offset_x);
		offset_x = clamp(offset_x, 0, 128);

		ImGui::InputInt("Offset Y", &offset_y);
		offset_y = clamp(offset_y, 0, 128);

		ImGui::InputInt("Spacing X", &spacing_x);
		spacing_x = clamp(spacing_x, 0, 128);

		ImGui::InputInt("Spacing Y", &spacing_y);
		spacing_y = clamp(spacing_y, 0, 128);

		ImGui::Combo("Pivot", &pivot, pivot_names, Pivot::COUNT);
		ImGui::InputInt("Layer", &layer);
		layer = clamp(layer, 0, 7);

		ImGui::InputInt("Depth", &depth);
		depth = clamp(depth, 0, 9999);

		ImGui::EndGroup();
	}
};

//-----------------------------------------------------------------------------
struct Inspector
{
	// Inspector
	char _name[1024];
	f32 _position[3];
	f32 _rotation[3];
	f32 _scale[3];
	char _sprite[1024];
	char _material[1024];
	bool _visible;
	char _state_machine[1024];
	bool _open;

	Inspector()
		: _visible(true)
		, _open(true)
	{
		memset(_name, 0, sizeof(_name));
		memset(_sprite, 0, sizeof(_sprite));
		memset(_material, 0, sizeof(_material));
		memset(_position, 0, sizeof(_position));
		memset(_rotation, 0, sizeof(_rotation));
		memset(_scale, 0, sizeof(_scale));
		memset(_state_machine, 0, sizeof(_state_machine));
	}

	void draw()
	{
		ImGui::Begin("Inspector", &_open);

		if (ImGui::TreeNodeEx("Unit", ImGuiTreeNodeFlags_DefaultOpen))
		{
			ImGui::InputText("Name", _name, sizeof(_name));
			ImGui::TreePop();
		}

		if (ImGui::TreeNodeEx("Transform", ImGuiTreeNodeFlags_DefaultOpen))
		{
			ImGui::InputFloat3("Position", _position);
			ImGui::InputFloat3("Rotation", _rotation);
			ImGui::InputFloat3("Scale", _scale);

			ImGui::TreePop();
		}

		if (ImGui::TreeNodeEx("Renderer", ImGuiTreeNodeFlags_DefaultOpen))
		{
			ImGui::InputText("Sprite", _sprite, sizeof(_sprite));
			ImGui::InputText("Material", _material, sizeof(_material));
			ImGui::Checkbox("Visible", &_visible);
			ImGui::TreePop();
		}

		if (ImGui::TreeNodeEx("Animation", ImGuiTreeNodeFlags_DefaultOpen))
		{
			ImGui::InputText("State Machine", _state_machine, sizeof(_state_machine));
			ImGui::TreePop();
		}

		ImGui::End();
	}
};

//-----------------------------------------------------------------------------
struct SceneView
{
	ImVec2 _origin;
	ImVec2 _size;
	ImVec2 _cursor;
	bool _open;

	SceneView()
		: _open(true)
	{
	}

	void draw()
	{
		ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
		ImGui::Begin("Scene View"
			, &_open
			, ImGuiWindowFlags_NoScrollbar
			| ImGuiWindowFlags_NoScrollWithMouse
			);

		_origin = ImGui::GetCursorScreenPos();

		uint16_t w, h;
		device()->resolution(w, h);
		bgfx::TextureHandle txh = device()->_pipeline->_buffers[0];
		CE_ENSURE(bgfx::isValid(txh));
		ImGui::Image((void*)(uintptr_t)txh.idx
			, ImVec2(w, h)
#if CROWN_PLATFORM_WINDOWS
			, ImVec2(0, 0)
			, ImVec2(1, 1)
#else
			, ImVec2(0, 1)
			, ImVec2(1, 0)
#endif // CROWN_PLATFORM_WINDOWS
		);

		ImVec2 mouse_pos_in_view = ImVec2(ImGui::GetIO().MousePos.x - _origin.x
			, ImGui::GetIO().MousePos.y - _origin.y
			);

		if (ImGui::IsWindowHovered()
			&& mouse_pos_in_view.x > 0
			&& mouse_pos_in_view.x < w
			&& mouse_pos_in_view.y > 0
			&& mouse_pos_in_view.y < h
			)
		{
			// Send all input to engine
			ImGui::CaptureMouseFromApp(false);
			ImGui::CaptureKeyboardFromApp(false);
		}
		else
		{
			// Send all input to imgui
			ImGui::CaptureMouseFromApp(true);
			ImGui::CaptureKeyboardFromApp(true);
		}

		_size = ImGui::GetWindowSize();
		_size.y -= ImGui::GetFrameHeight();

		ImGui::End();
		ImGui::PopStyleVar(2);
	}
};

//-----------------------------------------------------------------------------
struct SceneTree
{
	bool _open;

	SceneTree()
		: _open(true)
	{
	}

	void draw()
	{
		ImGui::Begin("Scene Tree", &_open);

		if (ImGui::TreeNodeEx("Units", ImGuiTreeNodeFlags_DefaultOpen))
		{
			if (ImGui::TreeNodeEx("Objects", ImGuiTreeNodeFlags_DefaultOpen))
			{
				for (int i = 0; i < 5; i++)
					if (ImGui::TreeNode((void*)(intptr_t)i, "Child %d", i))
					{
						ImGui::Text("blah blah");
						ImGui::SameLine();
						if (ImGui::SmallButton("print")) printf("Child %d pressed", i);
						ImGui::TreePop();
					}
				ImGui::TreePop();
			}

			if (ImGui::TreeNodeEx("Lights", ImGuiTreeNodeFlags_DefaultOpen))
			{
				// ShowHelpMarker("This is a more standard looking tree with selectable nodes.\nClick to select, Ctrl+Click to toggle, click on arrows or double-click to open.");
				static bool align_label_with_current_x_position = false;
				ImGui::Checkbox("Align label with current X position)", &align_label_with_current_x_position);
				ImGui::Text("Hello!");
				if (align_label_with_current_x_position)
					ImGui::Unindent(ImGui::GetTreeNodeToLabelSpacing());

				static int selection_mask = (1 << 2); // Dumb representation of what may be user-side selection state. You may carry selection state inside or outside your objects in whatever format you see fit.
				int node_clicked = -1;                // Temporary storage of what node we have clicked to process selection at the end of the loop. May be a pointer to your own node type, etc.
				ImGui::PushStyleVar(ImGuiStyleVar_IndentSpacing, ImGui::GetFontSize()*3); // Increase spacing to differentiate leaves from expanded contents.
				for (int i = 0; i < 6; i++)
				{
					// Disable the default open on single-click behavior and pass in Selected flag according to our selection state.
					ImGuiTreeNodeFlags node_flags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick | ((selection_mask & (1 << i)) ? ImGuiTreeNodeFlags_Selected : 0);
					if (i < 3)
					{
						// Node
						bool node_open = ImGui::TreeNodeEx((void*)(intptr_t)i, node_flags, "Selectable Node %d", i);
						if (ImGui::IsItemClicked())
							node_clicked = i;
						if (node_open)
						{
							ImGui::Text("Blah blah\nBlah Blah");
							ImGui::TreePop();
						}
					}
					else
					{
						// Leaf: The only reason we have a TreeNode at all is to allow selection of the leaf. Otherwise we can use BulletText() or TreeAdvanceToLabelPos()+Text().
						ImGui::TreeNodeEx((void*)(intptr_t)i, node_flags | ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen, "Selectable Leaf %d", i);
						if (ImGui::IsItemClicked())
							node_clicked = i;
					}
				}
				if (node_clicked != -1)
				{
					// Update selection state. Process outside of tree loop to avoid visual inconsistencies during the clicking-frame.
					if (ImGui::GetIO().KeyCtrl)
						selection_mask ^= (1 << node_clicked);          // Ctrl+click to toggle
					else //if (!(selection_mask & (1 << node_clicked))) // Depending on selection behavior you want, this commented bit preserve selection when clicking on item that is part of the selection
						selection_mask = (1 << node_clicked);           // Click to single-select
				}
				ImGui::PopStyleVar();
				if (align_label_with_current_x_position)
					ImGui::Indent(ImGui::GetTreeNodeToLabelSpacing());
				ImGui::TreePop();
			}
			ImGui::TreePop();
		}

		ImGui::End();
	}
};

//-----------------------------------------------------------------------------
struct SpriteAnimator
{
	bool _open;
	bool _add_animation_popup_open;

	Array<const char*> _entities;
	s32 _cur_entity;
	TextureResource* _texture;
	u32 _texture_width;
	u32 _texture_height;

	struct Frame
	{
		char	name[512];
		ImVec2	pivot;
		ImVec4	region;
	};
	Array<Frame> _frames;

	char _anim_name[512];
	f32 _anim_time;
	Array<const char*> _listbox_items;
	s32 _listbox_item_current;
	Array<Frame> _anim_preview_frames;

	f32 _delta;
	u32 current;

	Vector<DynamicString> file_list_sprites;

	FilesystemDisk* _fs;

	SpriteAnimator(const DynamicString& src_dir)
		: _open(false)
		, _add_animation_popup_open(false)
		, _entities(default_allocator())
		, _cur_entity(0)
		, _texture(nullptr)
		, _texture_width(0)
		, _texture_height(0)
		, _frames(default_allocator())
		, _anim_time(0.1f)
		, _listbox_items(default_allocator())
		, _listbox_item_current(0)
		, _anim_preview_frames(default_allocator())
		, _delta(0.0f)
		, current(0)
		, file_list_sprites(default_allocator())
	{
		memset(_anim_name, 0, sizeof(_anim_name));

		_fs = CE_NEW(default_allocator(), FilesystemDisk)(default_allocator());
		_fs->set_prefix(src_dir.c_str());

		get_sprites_list();
	}

	~SpriteAnimator()
	{
		CE_DELETE(default_allocator(), _fs);
	}

	ImVec2 pixel_to_uv(u32 tex_w, u32 tex_h, f32 x, f32 y)
	{
		ImVec2 uv;
		uv.x = (f32)x / (f32)tex_w;
		uv.y = (f32)y / (f32)tex_h;
		return uv;
	}

	void get_directory(DynamicString& dir, const char* path)
	{
		CE_ENSURE(NULL != path);

		const char* ls = strrchr(path, '/');
		u32 file_len = strlen32(ls+1);
		u32 dir_len = strlen32(path) - file_len;

		char buff[1024];
		memcpy(buff, path, dir_len);
		buff[dir_len] = '\0';

		path::reduce(dir, buff);
	}

	void save_sprite_animation()
	{
		TempAllocator4096 ta;

		StringStream ss(ta);
		ss << "frames = [ ";
		for (u32 i = 0; i < array::size(_listbox_items); i++)
		{
			ss << _listbox_items[i] + 7 << " ";
		}
		ss << "]\n";
		ss << "total_time = ";
		ss << _anim_time;

		DynamicString dir(ta);
		get_directory(dir, _entities[_cur_entity]);
		DynamicString file_name(ta);
		path::join(file_name, dir.c_str(), _anim_name);
		file_name += ".sprite_animation";
		File* f = _fs->open(file_name.c_str(), FileOpenMode::WRITE);
		f->write(string_stream::c_str(ss), strlen32(string_stream::c_str(ss)));
		f->close();
	}

	void get_files_list(Vector<DynamicString>& out, const char* path="")
	{
		TempAllocator4096 ta;
		Vector<DynamicString> files(ta);

		_fs->list_files(path, files);

		for (u32 i = 0; i < vector::size(files); i++)
		{
			DynamicString join(ta);
			path::join(join, path, files[i].c_str());
			join = join.c_str()[0] == '/' ? join.c_str()+1 : join.c_str();

			if (_fs->is_directory(join.c_str()))
			{
				get_files_list(out, join.c_str());
			}
			else
			{
				vector::push_back(out, join);
			}
		}
	}

	void get_sprites_list()
	{
		TempAllocator4096 ta;
		Vector<DynamicString> files(ta);
		get_files_list(files);

		for (DynamicString* f = vector::begin(files); f != vector::end(files); f++)
			if (f->has_suffix(".sprite"))
				array::push_back(_entities, (const char*) strdup(f->c_str()));
	}

	void draw()
	{
		ImGui::Begin("Animator", &_open);

		if (_texture)
		{
			Frame f = _frames[0];
			ImVec2 start = pixel_to_uv(_texture_width, _texture_height, f.region.x, f.region.y);
			ImVec2 end = pixel_to_uv(_texture_width, _texture_height, f.region.x+f.region.z, f.region.y+f.region.w);
			ImGui::Image((void*)(uintptr_t)_texture->handle.idx
				, ImVec2(f.region.z, f.region.w)
				, start
				, end
				, ImColor(255, 255, 255, 55)
			);
		}

		if (ImGui::Combo("Entities", &_cur_entity, (const char* const*) array::begin(_entities), array::size(_entities)))
		{
			array::clear(_frames);

			const char* sprite = _entities[_cur_entity];
			u32 sprite_len = strlen32(sprite);
			char entity[1024];
			strncpy(entity, sprite, sizeof(entity)-1);
			entity[sprite_len-7] = '\0'; // remove ".sprite"

			ResourceManager* resman = device()->_resource_manager;
			_texture = (TextureResource*)resman->get(RESOURCE_TYPE_TEXTURE, StringId64(entity));

			File* file = _fs->open(sprite, FileOpenMode::READ);
			const u32 size = file->size();
			Buffer buf(default_allocator());
			array::resize(buf, size);
			file->read(array::begin(buf), size);
			_fs->close(*file);

			JsonObject obj(default_allocator());
			JsonArray list(default_allocator());
			sjson::parse(obj, buf);
			sjson::parse_array(list, obj["frames"]);
			_texture_width = sjson::parse_int(obj["width"]);
			_texture_height = sjson::parse_int(obj["height"]);
			for (u32 i = 0; i < array::size(list); i++)
			{
				JsonObject frame(default_allocator());
				DynamicString name(default_allocator());
				JsonArray pivot(default_allocator());
				JsonArray region(default_allocator());
				sjson::parse_object(frame, list[i]);
				sjson::parse_array(pivot, frame["pivot"]);
				sjson::parse_array(region, frame["region"]);

				Frame f;
				sjson::parse_string(name, frame["name"]);
				strncpy(f.name, name.c_str(), name.length());
				f.name[name.length()] = '\0';
				f.pivot.x = sjson::parse_float(pivot[0]);
				f.pivot.y = sjson::parse_float(pivot[1]);
				f.region.x = sjson::parse_float(region[0]);
				f.region.y = sjson::parse_float(region[1]);
				f.region.z = sjson::parse_float(region[2]);
				f.region.w = sjson::parse_float(region[3]);

				array::push_back(_frames, f);
			}
		}

		if (ImGui::Button("Add animation", ImVec2(100, 25)))
		{
			_add_animation_popup_open = true;
		}

		if (_add_animation_popup_open)
		{
			ImGui::OpenPopup("Add animation");
		}

		if (ImGui::BeginPopup("Add animation"))
		{
			ImGui::InputText("Name", _anim_name, sizeof(_anim_name));
			ImGui::InputFloat("Time", &_anim_time, 0.1f, 0.1f);
			ImGui::ListBox("Animation Frames", &_listbox_item_current, (const char* const*)array::begin(_listbox_items), array::size(_listbox_items));
			if (ImGui::Button("Clear Frames", ImVec2(100.0f, 25.0f)))
			{
				array::clear(_listbox_items);
				array::clear(_anim_preview_frames);
				_delta = 0.0f;
				current = 0;
			}

			if (array::size(_anim_preview_frames) > 0)
			{
				_delta += 1.0f/60.0f;
				if (_delta >= _anim_time/array::size(_anim_preview_frames))
				{
					_delta = 0;
					current++;
					if (current >= array::size(_anim_preview_frames))
						current = 0;
				}

				Frame f = _anim_preview_frames[current];
				ImVec2 start = pixel_to_uv(_texture_width, _texture_height, f.region.x, f.region.y);
				ImVec2 end = pixel_to_uv(_texture_width, _texture_height, f.region.x+f.region.z, f.region.y+f.region.w);
				ImGui::Image(
					  (void*)(uintptr_t)_texture->handle.idx
					, ImVec2(f.region.z, f.region.w)
					, start
					, end
					, ImColor(255, 255, 255, 55)
					);
			}

			ImGui::Separator();

			for (u32 i = 0; i < array::size(_frames); i++)
			{
				Frame f = _frames[i];
				ImVec2 start = pixel_to_uv(_texture_width, _texture_height, f.region.x, f.region.y);
				ImVec2 end = pixel_to_uv(_texture_width, _texture_height, f.region.x+f.region.z, f.region.y+f.region.w);

				ImGui::SameLine();
				if (i % 9 == 0) ImGui::NewLine();
				ImGui::BeginGroup();
				ImGui::Image(
					  (void*)(uintptr_t)_texture->handle.idx
					, ImVec2(f.region.z, f.region.w)
					, start
					, end
					, ImColor(255, 255, 255, 55)
					);
				ImGui::NewLine();
				if (ImGui::Button(_frames[i].name, ImVec2(100.0f, 25.0f)))
				{
					array::push_back(_listbox_items, (const char*) strdup(_frames[i].name));
					array::push_back(_anim_preview_frames, _frames[i]);
				}
				ImGui::EndGroup();
			}

			if (ImGui::Button("Save", ImVec2(100, 25)))
			{
				save_sprite_animation();
				ImGui::CloseCurrentPopup();
				_add_animation_popup_open = false;
			}
			ImGui::SameLine();
			if (ImGui::Button("Cancel", ImVec2(100, 25)))
			{
				ImGui::CloseCurrentPopup();
				_add_animation_popup_open = false;
			}

			ImGui::EndPopup();
		}

		ImGui::End();
	}
};

//-----------------------------------------------------------------------------
struct LevelEditor
{
	DynamicString _source_dir;

	// FX
	TextureResource* tool_move_texture;
	TextureResource* tool_place_texture;
	TextureResource* tool_rotate_texture;
	TextureResource* tool_scale_texture;
	TextureResource* reference_world_texture;
	TextureResource* reference_local_texture;
	TextureResource* axis_local_texture;
	TextureResource* axis_world_texture;
	TextureResource* snap_to_grid_texture;

	// State
	f32 _grid_size;
	f32 _rotation_snap;
	bool _show_grid;
	bool _snap_to_grid;
	bool _debug_render_world;
	bool _debug_physics_world;
	tool::ToolType::Enum _tool_type;
	tool::SnapMode::Enum _snap_mode;
	tool::ReferenceSystem::Enum _reference_system;

	ImVec2 _main_menu_pos;
	ImVec2 _main_menu_size;

	Console _console;
	Inspector _inspector;
	SceneView _scene_view;
	SceneTree _scene_tree;
	SpriteAnimator _animator;
	SpriteImporter _sprite_importer;

	bool _fullscreen;

	LevelEditor(const DynamicString& source_dir)
		: _source_dir(default_allocator())
		, _grid_size(1.0f)
		, _rotation_snap(15)
		, _show_grid(true)
		, _snap_to_grid(true)
		, _debug_render_world(false)
		, _debug_physics_world(false)
		, _tool_type(tool::ToolType::MOVE)
		, _snap_mode(tool::SnapMode::RELATIVE)
		, _reference_system(tool::ReferenceSystem::LOCAL)

		, _main_menu_pos(0, 0)
		, _main_menu_size(0, 0)

		, _animator(source_dir)

		, _fullscreen(false)
	{
		ResourceManager* resman = device()->_resource_manager;
		tool_move_texture       = (TextureResource*)resman->get(RESOURCE_TYPE_TEXTURE, STRING_ID_64("core/editors/gui/tool-move", 0x57f16b5c703944e8));
		tool_place_texture      = (TextureResource*)resman->get(RESOURCE_TYPE_TEXTURE, STRING_ID_64("core/editors/gui/tool-place", 0xf33a70241fe20ffd));
		tool_rotate_texture     = (TextureResource*)resman->get(RESOURCE_TYPE_TEXTURE, STRING_ID_64("core/editors/gui/tool-rotate", 0x59bfc47bde2b2072));
		tool_scale_texture      = (TextureResource*)resman->get(RESOURCE_TYPE_TEXTURE, STRING_ID_64("core/editors/gui/tool-scale", 0x73d5bcd35a65d39c));
		reference_world_texture = (TextureResource*)resman->get(RESOURCE_TYPE_TEXTURE, STRING_ID_64("core/editors/gui/reference-world", 0xe072b9ad48895d31));
		reference_local_texture = (TextureResource*)resman->get(RESOURCE_TYPE_TEXTURE, STRING_ID_64("core/editors/gui/reference-local", 0x22153239997ae30a));
		axis_local_texture      = (TextureResource*)resman->get(RESOURCE_TYPE_TEXTURE, STRING_ID_64("core/editors/gui/axis-local", 0xb3b5b50a9cecce48));
		axis_world_texture      = (TextureResource*)resman->get(RESOURCE_TYPE_TEXTURE, STRING_ID_64("core/editors/gui/axis-world", 0xd10f6f1a9a680032));
		snap_to_grid_texture    = (TextureResource*)resman->get(RESOURCE_TYPE_TEXTURE, STRING_ID_64("core/editors/gui/snap-to-grid", 0xf8831d3758d30394));

		imgui_create();
	}

	~LevelEditor()
	{
		imgui_destroy();
	}

	void update(f32 dt)
	{
		CE_UNUSED(dt);

		static f32 last_w = 0.0f;
		static f32 last_h = 0.0f;
		if (last_w != _scene_view._size.x || last_h != _scene_view._size.y)
		{
			last_w = _scene_view._size.x;
			last_h = _scene_view._size.y;
			device()->_width  = u16(_scene_view._size.x != 0.0f ? _scene_view._size.x : 128.0f);
			device()->_height = u16(_scene_view._size.y != 0.0f ? _scene_view._size.y : 128.0f);
		}

		u32 message_count = 0;

		// Receive response from engine
		for (;;)
		{
			u32 msg_len = 0;
			ReadResult rr = _client.read_nonblock(&msg_len, sizeof(msg_len));

			if (rr.error == ReadResult::WOULDBLOCK)
				break;

			if (ReadResult::SUCCESS == rr.error)
			{
				char msg[8192];
				rr = _client.read(msg, msg_len);
				msg[msg_len] = '\0';
				message_count++;
				// logi(LEVEL_EDITOR, "count: %d", message_count);
				if (ReadResult::SUCCESS == rr.error)
				{
					TempAllocator4096 ta;
					JsonObject obj(ta);
					DynamicString type(ta);
					json::parse(obj, msg);
					json::parse_string(type, obj["type"]);

					if (type == "message")
					{
						DynamicString severity(ta);
						DynamicString message(ta);
						json::parse_string(severity, obj["severity"]);
						json::parse_string(message, obj["message"]);

						LogSeverity::Enum ls = LogSeverity::COUNT;
						if (strcmp("info", severity.c_str()) == 0)
							ls = LogSeverity::LOG_INFO;
						else if (strcmp("warning", severity.c_str()) == 0)
							ls = LogSeverity::LOG_WARN;
						else if (strcmp("error", severity.c_str()) == 0)
							ls = LogSeverity::LOG_ERROR;
						else
							CE_FATAL("Unknown severity");

						_console.add_log(ls, message.c_str());
					}
					else if (type == "error")
					{
						DynamicString message(ta);
						json::parse_string(message, obj["message"]);

						_console.add_log(LogSeverity::LOG_ERROR, message.c_str());
					}
					else
					{
						_console.add_log(LogSeverity::LOG_ERROR, "Unknown message type");
					}

					console_scroll_to_bottom(_console);
				}
			}
		}

		imgui_begin_frame(VIEW_IMGUI, _width, _height);

		ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_None;
		// We are using the ImGuiWindowFlags_NoDocking flag to make the parent window not dockable into,
		// because it would be confusing to have two docking targets within each others.
		ImGuiWindowFlags window_flags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;
		ImGuiViewport* viewport = ImGui::GetMainViewport();
		ImGui::SetNextWindowPos(viewport->Pos);
		ImGui::SetNextWindowSize(viewport->Size);
		ImGui::SetNextWindowViewport(viewport->ID);
		ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
		ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
		window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
		window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;

		// When using ImGuiDockNodeFlags_PassthruCentralNode, DockSpace() will render our background and handle the pass-thru hole, so we ask Begin() to not render a background.
		if (dockspace_flags & ImGuiDockNodeFlags_PassthruCentralNode)
		    window_flags |= ImGuiWindowFlags_NoBackground;

		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
		ImGui::Begin("DockSpace Demo", NULL, window_flags);
		ImGui::PopStyleVar(3);

		// DockSpace
		ImGuiID dockspace_id = ImGui::GetID("MyDockSpace");
		ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), dockspace_flags);

		main_menu_bar();
		ImGui::End();

		if (_scene_view._open)
		{
			_scene_view.draw();

			// Draw toolbar overlay
			ImVec2 window_pos;
			window_pos.x = _scene_view._origin.x + 6.0f;
			window_pos.y = _scene_view._origin.y + 6.0f;
			ImGui::SetNextWindowPos(window_pos, ImGuiCond_Always);
			ImGui::SetNextWindowBgAlpha(0.35f);
			if (ImGui::Begin("Toobar Overlay"
				, NULL
				, ImGuiWindowFlags_NoMove
				| ImGuiWindowFlags_NoDecoration
				| ImGuiWindowFlags_AlwaysAutoResize
				| ImGuiWindowFlags_NoSavedSettings
				| ImGuiWindowFlags_NoFocusOnAppearing
				| ImGuiWindowFlags_NoNav
				))
			{
				ImGui::BeginGroup();
				if (ImGui::ImageButton(tool_place_texture->handle, ImVec2(16, 16)))
				{
					_tool_type = tool::ToolType::PLACE;
					tool_send_state();
				}

				if (ImGui::ImageButton(tool_move_texture->handle, ImVec2(16, 16)))
				{
					_tool_type = tool::ToolType::MOVE;
					tool_send_state();
				}

				if (ImGui::ImageButton(tool_rotate_texture->handle, ImVec2(16, 16)))
				{
					_tool_type = tool::ToolType::ROTATE;
					tool_send_state();
				}

				if (ImGui::ImageButton(tool_scale_texture->handle, ImVec2(16, 16)))
				{
					_tool_type = tool::ToolType::SCALE;
					tool_send_state();
				}

				ImGui::Separator();

				if (ImGui::ImageButton(axis_local_texture->handle, ImVec2(16, 16)))
				{
					_reference_system = tool::ReferenceSystem::LOCAL;
					tool_send_state();
				}

				if (ImGui::ImageButton(axis_world_texture->handle, ImVec2(16, 16)))
				{
					_reference_system = tool::ReferenceSystem::WORLD;
					tool_send_state();
				}

				ImGui::Separator();

				if (ImGui::ImageButton(reference_world_texture->handle, ImVec2(16, 16)))
				{
					_snap_mode = tool::SnapMode::RELATIVE;
					tool_send_state();
				}

				if (ImGui::ImageButton(reference_local_texture->handle, ImVec2(16, 16)))
				{
					_snap_mode = tool::SnapMode::ABSOLUTE;
					tool_send_state();
				}

				ImGui::Separator();

				if (ImGui::ImageButton(snap_to_grid_texture->handle, ImVec2(16, 16)))
				{
					_snap_to_grid = !_snap_to_grid;
					tool_send_state();
				}

				ImGui::EndGroup();
			}
			ImGui::End();
		}

		if (_scene_tree._open)
			_scene_tree.draw();

		if (_inspector._open)
			_inspector.draw();

		if (_console._open)
			console_draw(_console, _client);

		if (_animator._open)
			_animator.draw();

		// _sprite_importer.draw();

		imgui_end_frame();
	}

	void send_command(StringStream& ss)
	{
		console_send_script(_client, string_stream::c_str(ss));
	}

	void tool_send_state()
	{
		TempAllocator256 ta;
		StringStream ss(ta);
		tool::set_grid_size(ss, _grid_size);
		tool::set_rotation_snap(ss, _rotation_snap);
		tool::enable_show_grid(ss, _show_grid);
		tool::enable_snap_to_grid(ss, _snap_to_grid);
		tool::enable_debug_render_world(ss, _debug_render_world);
		tool::enable_debug_physics_world(ss, _debug_physics_world);
		tool::set_tool_type(ss, _tool_type);
		tool::set_snap_mode(ss, _snap_mode);
		tool::set_reference_system(ss, _reference_system);
		send_command(ss);
	}

	void format_time(uint64_t time, char* str, size_t size)
	{
#if CROWN_PLATFORM_WINDOWS
		// SYSTEMTIME st;
		// FileTimeToSystemTime(&time, &st);
		// sprintf(str, "%d/%d/%d %d:%d"
		// 	, st.wDay
		// 	, st.wMonth
		// 	, st.wYear
		// 	, st.wHour
		// 	, st.wMinute);
#else
		struct tm lt;
		localtime_r((time_t*)&time, &lt);
		strftime(str, size, "%d/%m/%Y %H:%M", &lt);
#endif
	}

	void main_menu_bar()
	{
		// Main Menu
		if (ImGui::BeginMainMenuBar())
		{
			if (ImGui::BeginMenu("File"))
			{
				if (ImGui::MenuItem("New"))
				{

				}
				if (ImGui::MenuItem("Open", "Ctrl+O"))
				{
					nfdchar_t *out_path = NULL;
					nfdresult_t result = NFD_OpenDialog(NULL, NULL, &out_path);

					if ( result == NFD_OKAY )
					{
						logi(LEVEL_EDITOR, "Success!");
						logi(LEVEL_EDITOR, out_path);
						free(out_path);
					}
					else if ( result == NFD_CANCEL )
					{
						logi(LEVEL_EDITOR, "User pressed cancel.");
					}
					else
					{
						loge(LEVEL_EDITOR, "Error: %s\n", NFD_GetError());
					}
				}
				if (ImGui::MenuItem("Save", "Ctrl+S"))
				{
				}
				if (ImGui::MenuItem("Save As..."))
				{
				}
				ImGui::Separator();
				if (ImGui::MenuItem("Quit", "Ctrl+Q", false, true))
				{
					TempAllocator64 ta;
					StringStream ss(ta);
					tool::device_quit(ss);
					send_command(ss);
				}
				ImGui::EndMenu();
			}
			if (ImGui::BeginMenu("Edit"))
			{
				if (ImGui::MenuItem("Undo", "Ctrl+Z")) {}
				if (ImGui::MenuItem("Redo", "Ctrl+Y")) {}
				ImGui::EndMenu();
			}
			if (ImGui::BeginMenu("Create"))
			{
				TempAllocator256 ta;
				StringStream ss(ta);

				if (ImGui::BeginMenu("Primitives"))
				{
					if (ImGui::MenuItem("Cube", NULL, false, true))
					{
						_tool_type = tool::ToolType::PLACE;
						tool_send_state();
						tool::set_placeable(ss, "unit", "core/units/primitives/cube");
						send_command(ss);
					}
					if (ImGui::MenuItem("Sphere"))
					{
						_tool_type = tool::ToolType::PLACE;
						tool_send_state();
						tool::set_placeable(ss, "unit", "core/units/primitives/sphere");
						send_command(ss);
					}
					if (ImGui::MenuItem("Cone"))
					{
						_tool_type = tool::ToolType::PLACE;
						tool_send_state();
						tool::set_placeable(ss, "unit", "core/units/primitives/cone");
						send_command(ss);
					}
					if (ImGui::MenuItem("Cylinder"))
					{
						_tool_type = tool::ToolType::PLACE;
						tool_send_state();
						tool::set_placeable(ss, "unit", "core/units/primitives/cylinder");
						send_command(ss);
					}
					if (ImGui::MenuItem("Plane"))
					{
						_tool_type = tool::ToolType::PLACE;
						tool_send_state();
						tool::set_placeable(ss, "unit", "core/units/primitives/plane");
						send_command(ss);
					}
					ImGui::EndMenu();
				}

				if (ImGui::MenuItem("Camera"))
				{
					_tool_type = tool::ToolType::PLACE;
					tool_send_state();
					tool::set_placeable(ss, "unit", "core/units/camera");
					send_command(ss);
				}
				if (ImGui::MenuItem("Light"))
				{
					_tool_type = tool::ToolType::PLACE;
					tool_send_state();
					tool::set_placeable(ss, "unit", "core/units/light");
					send_command(ss);
				}
				if (ImGui::MenuItem("Sound"))
				{
					_tool_type = tool::ToolType::PLACE;
					tool_send_state();
					tool::set_placeable(ss, "sound", "core/units/camera");
					send_command(ss);
				}
				ImGui::EndMenu();
			}
			if (ImGui::BeginMenu("Scene View"))
			{
				TempAllocator256 ta;
				StringStream ss(ta);

				if (ImGui::BeginMenu("Camera View"))
				{
					if (ImGui::MenuItem("Perspective"))
					{
						tool::camera_view_perspective(ss);
						send_command(ss);
					}
					if (ImGui::MenuItem("Front"))
					{
						tool::camera_view_front(ss);
						send_command(ss);
					}
					if (ImGui::MenuItem("Back"))
					{
						tool::camera_view_back(ss);
						send_command(ss);
					}
					if (ImGui::MenuItem("Right"))
					{
						tool::camera_view_right(ss);
						send_command(ss);
					}
					if (ImGui::MenuItem("Left"))
					{
						tool::camera_view_left(ss);
						send_command(ss);
					}
					if (ImGui::MenuItem("Top"))
					{
						tool::camera_view_top(ss);
						send_command(ss);
					}
					if (ImGui::MenuItem("Bottom"))
					{
						tool::camera_view_bottom(ss);
						send_command(ss);
					}
					ImGui::EndMenu();
				}
				ImGui::Separator();
				if (ImGui::MenuItem("Show Grid", NULL, _show_grid))
				{
					_show_grid = !_show_grid;
					tool_send_state();
				}
				if (ImGui::SliderFloat("Grid Size", &_grid_size, 0.1f, 5.0f))
				{
					tool_send_state();
				}
				if (ImGui::SliderFloat("Snap Rot", &_rotation_snap, 1, 180))
				{
					tool_send_state();
				}
				ImGui::Separator();
				if (ImGui::MenuItem("Toggle Full Screen", "F11"))
				{
					_fullscreen = !_fullscreen;
					device()->_window->set_fullscreen(_fullscreen);
				}
				ImGui::EndMenu();
			}
			if (ImGui::BeginMenu("Windows"))
			{
				if (ImGui::MenuItem("Scene"))
				{
					_scene_view._open = true;
				}
				if (ImGui::MenuItem("Scene Tree"))
				{
					_scene_tree._open = true;
				}
				if (ImGui::MenuItem("Inspector"))
				{
					_inspector._open = true;
				}
				if (ImGui::MenuItem("Console"))
				{
					_console._open = true;
				}
				if (ImGui::MenuItem("Animator"))
				{
					_animator._open = true;
				}

				ImGui::EndMenu();
			}
			if (ImGui::BeginMenu("Test"))
			{
				if (ImGui::MenuItem("Test Level"))
					start_game(StartGame::TEST, _project->_data_dir.c_str());
				if (ImGui::MenuItem("Test Game"))
					start_game(StartGame::NORMAL, _project->_data_dir.c_str());
				if (ImGui::MenuItem("Stop"))
					stop_game();

				ImGui::EndMenu();
			}
			if (ImGui::BeginMenu("Help"))
			{
				ImGui::MenuItem("About", "");
				ImGui::EndMenu();
			}

			_main_menu_pos = ImGui::GetWindowPos();
			_main_menu_size = ImGui::GetWindowSize();

			ImGui::EndMainMenuBar();
		}
	}
};

void tool_init()
{
	const DeviceOptions& opt = device()->_options;
	_project = CE_NEW(default_allocator(), Project)(default_allocator());
	_project->_data_dir = opt._data_dir;
	_editor = CE_NEW(default_allocator(), LevelEditor)(opt._source_dir);
	_client.connect(IP_ADDRESS_LOOPBACK, CROWN_DEFAULT_CONSOLE_PORT);
}

void tool_update(f32 dt)
{
	_editor->update(dt);
}

void tool_shutdown()
{
	_client.close();
	CE_DELETE(default_allocator(), _editor);
	CE_DELETE(default_allocator(), _project);

	stop_game();
}

extern bool next_event(OsEvent& ev);

bool tool_process_events()
{
	ImGuiIO& io = ImGui::GetIO();

	bool exit = false;
	bool reset = false;

	TempAllocator4096 ta;
	StringStream ss(ta);

	OsEvent event;
	while (next_event(event))
	{
		switch (event.type)
		{
		case OsEventType::BUTTON:
			switch (event.button.device_id)
			{
			case crown::InputDeviceType::KEYBOARD:
				io.KeysDown[event.button.button_num] = event.button.pressed;

				io.KeyCtrl = io.KeysDown[crown::KeyboardButton::CTRL_LEFT] || io.KeysDown[crown::KeyboardButton::CTRL_RIGHT];
				io.KeyShift = io.KeysDown[crown::KeyboardButton::SHIFT_LEFT] || io.KeysDown[crown::KeyboardButton::SHIFT_RIGHT];
				io.KeyAlt = io.KeysDown[crown::KeyboardButton::ALT_LEFT] || io.KeysDown[crown::KeyboardButton::ALT_RIGHT];
				io.KeySuper = io.KeysDown[crown::KeyboardButton::SUPER_LEFT] || io.KeysDown[crown::KeyboardButton::SUPER_RIGHT];

				if (!io.WantCaptureKeyboard)
				{
					if (event.button.pressed)
					{
						if (event.button.button_num == crown::KeyboardButton::W)
							tool::key_down(ss, "w");
						if (event.button.button_num == crown::KeyboardButton::A)
							tool::key_down(ss, "a");
						if (event.button.button_num == crown::KeyboardButton::S)
							tool::key_down(ss, "s");
						if (event.button.button_num == crown::KeyboardButton::D)
							tool::key_down(ss, "d");
						if (event.button.button_num == crown::KeyboardButton::CTRL_LEFT)
							tool::key_down(ss, "ctrl_left");
						if (event.button.button_num == crown::KeyboardButton::SHIFT_LEFT)
							tool::key_down(ss, "shift_left");
						if (event.button.button_num == crown::KeyboardButton::ALT_LEFT)
							tool::key_down(ss, "alt_left");
					}
					else
					{
						if (event.button.button_num == crown::KeyboardButton::W)
							tool::key_up(ss, "w");
						if (event.button.button_num == crown::KeyboardButton::A)
							tool::key_up(ss, "a");
						if (event.button.button_num == crown::KeyboardButton::S)
							tool::key_up(ss, "s");
						if (event.button.button_num == crown::KeyboardButton::D)
							tool::key_up(ss, "d");
						if (event.button.button_num == crown::KeyboardButton::CTRL_LEFT)
							tool::key_up(ss, "ctrl_left");
						if (event.button.button_num == crown::KeyboardButton::SHIFT_LEFT)
							tool::key_up(ss, "shift_left");
						if (event.button.button_num == crown::KeyboardButton::ALT_LEFT)
							tool::key_up(ss, "alt_left");
					}
				}
				break;

			case crown::InputDeviceType::MOUSE:
				io.MouseDown[event.button.button_num] = event.button.pressed;

				if (!io.WantCaptureMouse)
				{
					ImVec2& cursor = _editor->_scene_view._cursor;
					cursor.x = io.MousePos.x - _editor->_scene_view._origin.x;
					cursor.y = io.MousePos.y - _editor->_scene_view._origin.y;

					tool::set_mouse_state(ss
						, cursor.x
						, cursor.y
						, io.MouseDown[MouseButton::LEFT]
						, io.MouseDown[MouseButton::MIDDLE]
						, io.MouseDown[MouseButton::RIGHT]
						);

					if (event.button.pressed)
					{
						if (io.KeysDown[crown::KeyboardButton::ALT_LEFT])
						{
							if (event.button.button_num == crown::MouseButton::LEFT)
								ss << "LevelEditor:camera_drag_start('tumble')";
							if (event.button.button_num == crown::MouseButton::MIDDLE)
								ss << "LevelEditor:camera_drag_start('track')";
							if (event.button.button_num == crown::MouseButton::RIGHT)
								ss << "LevelEditor:camera_drag_start('dolly')";
						}
						else
						{
							tool::mouse_down(ss, cursor.x, cursor.y);
						}
					}
					else
					{
						if (io.KeysDown[crown::KeyboardButton::ALT_LEFT])
						{
							if (event.button.button_num != crown::MouseButton::LEFT
								|| event.button.button_num != crown::MouseButton::MIDDLE
								|| event.button.button_num != crown::MouseButton::RIGHT
								)
							{
								ss << "LevelEditor:camera_drag_start('idle')";
							}
						}
						else
						{
							tool::mouse_up(ss, cursor.x, cursor.y);
						}
					}
				}
				break;
			}
			break;

		case OsEventType::AXIS:
			switch(event.axis.device_id)
			{
			case InputDeviceType::MOUSE:
				switch(event.axis.axis_num)
				{
				case crown::MouseAxis::CURSOR:
					io.MousePos = ImVec2(event.axis.axis_x, event.axis.axis_y);

					if (!io.WantCaptureMouse)
					{
						ImVec2& cursor = _editor->_scene_view._cursor;
						cursor.x = io.MousePos.x - _editor->_scene_view._origin.x;
						cursor.y = io.MousePos.y - _editor->_scene_view._origin.y;

						tool::set_mouse_state(ss
							, cursor.x
							, cursor.y
							, io.MouseDown[MouseButton::LEFT]
							, io.MouseDown[MouseButton::MIDDLE]
							, io.MouseDown[MouseButton::RIGHT]
							);
					}
					break;

				case crown::MouseAxis::WHEEL:
					io.MouseWheel += event.axis.axis_y;

					if (!io.WantCaptureMouse)
						tool::mouse_wheel(ss, io.MouseWheel);
					break;
				}
			}
			break;

		case OsEventType::TEXT:
			if (event.text.utf8[0] < 0x7f)
				io.AddInputCharactersUTF8((const char*)event.text.utf8);
			break;

		case OsEventType::RESOLUTION:
			_width  = event.resolution.width;
			_height = event.resolution.height;
			reset   = true;
			break;

		case OsEventType::EXIT:
			exit = true;
			break;

		default:
			break;
		}

	}

	if (array::size(ss) > 0)
		_editor->send_command(ss);

	bool vsync = true;
	if (reset)
		bgfx::reset(_width, _height, (vsync ? BGFX_RESET_VSYNC : BGFX_RESET_NONE));

	return exit;
}

} // namespace crown

#endif // CROWN_TOOLS
