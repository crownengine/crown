/*
 * Copyright (c) 2012-2026 Daniele Bartolini et al.
 * SPDX-License-Identifier: MIT
 */

#include "config.h"
#include "core/containers/array.inl"
#include "core/debug/debug.h"
#include "core/filesystem/file.h"
#include "core/filesystem/filesystem.h"
#include "core/filesystem/filesystem_apk.h"
#include "core/filesystem/filesystem_disk.h"
#include "core/json/json_object.inl"
#include "core/json/sjson.h"
#include "core/list.inl"
#include "core/math/constants.h"
#include "core/math/matrix4x4.inl"
#include "core/math/vector3.inl"
#include "core/memory/globals.h"
#include "core/memory/proxy_allocator.h"
#include "core/memory/temp_allocator.inl"
#include "core/network/ip_address.h"
#include "core/network/socket.h"
#include "core/option.inl"
#include "core/os.h"
#include "core/profiler.h"
#include "core/strings/dynamic_string.inl"
#include "core/strings/string.inl"
#include "core/strings/string_id.inl"
#include "core/strings/string_stream.inl"
#include "core/time.h"
#include "core/types.h"
#include "device/console_server.h"
#include "device/device.h"
#include "device/graph.h"
#include "device/input_device.h"
#include "device/input_manager.h"
#include "device/log.h"
#include "device/pipeline.h"
#include "lua/lua_environment.h"
#include "lua/lua_stack.inl"
#include "resource/resource_id.inl"
#include "resource/resource_loader.h"
#include "resource/resource_manager.h"
#include "resource/resource_package.h"
#include "world/animation_state_machine.h"
#include "world/material_manager.h"
#include "world/physics.h"
#include "world/shader_manager.h"
#include "world/sound_world.h"
#include "world/unit_manager.h"
#include "world/world.h"
#include <bgfx/bgfx.h>
#include <bimg/bimg.h>
#include <bx/allocator.h>
#include <bx/error.h>
#include <bx/error.h>
#include <bx/file.h>
#include <bx/math.h>
#if CROWN_PLATFORM_EMSCRIPTEN
	#include <emscripten/emscripten.h>
#endif

#define CROWN_MAX_SUBSYSTEMS_HEAP (8*1024*1024)

LOG_SYSTEM(DEVICE, "device")

namespace crown
{
extern bool next_event(OsEvent &ev);

#define RESOURCE_TYPE(type_name)                            \
	namespace type_name##_resource_internal                 \
	{                                                       \
		extern void *load(File &, Allocator &);             \
		extern void online(StringId64, ResourceManager &);  \
		extern void offline(StringId64, ResourceManager &); \
		extern void unload(Allocator &, void *);            \
	}

RESOURCE_TYPE(config)
RESOURCE_TYPE(font)
RESOURCE_TYPE(level)
RESOURCE_TYPE(material)
RESOURCE_TYPE(mesh)
RESOURCE_TYPE(mesh_skeleton)
RESOURCE_TYPE(mesh_animation)
RESOURCE_TYPE(package)
RESOURCE_TYPE(physics_config)
RESOURCE_TYPE(render_config)
RESOURCE_TYPE(lua)
RESOURCE_TYPE(shader)
RESOURCE_TYPE(sound)
RESOURCE_TYPE(sprite)
RESOURCE_TYPE(sprite_animation)
RESOURCE_TYPE(state_machine)
RESOURCE_TYPE(texture)
RESOURCE_TYPE(unit)

#undef RESOURCE_TYPE

struct BgfxCallback : public bgfx::CallbackI
{
	DynamicString _screenshot_path;
	std::atomic_int _screenshot_ready;

	explicit BgfxCallback(Allocator &a)
		: _screenshot_path(a)
		, _screenshot_ready(0)
	{
	}

	virtual void fatal(const char *_filePath, uint16_t _line, bgfx::Fatal::Enum _code, const char *_str) override
	{
		CE_UNUSED_4(_filePath, _line, _code, _str);
		error::abort("%s (code 0x%08x).\n", _str, _code);
	}

	virtual void traceVargs(const char *_filePath, u16 _line, const char *_format, va_list _argList) override
	{
		CE_UNUSED_2(_filePath, _line);
		char buf[2048];
		strncpy(buf, _format, sizeof(buf) - 1);
		buf[strlen32(buf) - 1] = '\0'; // Remove trailing newline
		vlogi(DEVICE, buf, _argList);
	}

	virtual void profilerBegin(const char *_name, uint32_t _abgr, const char *_filePath, uint16_t _line) override
	{
		CE_UNUSED_4(_name, _abgr, _filePath, _line);
	}

	virtual void profilerBeginLiteral(const char *_name, uint32_t _abgr, const char *_filePath, uint16_t _line) override
	{
		CE_UNUSED_4(_name, _abgr, _filePath, _line);
	}

	virtual void profilerEnd() override
	{
	}

	virtual u32 cacheReadSize(u64 _id) override
	{
		CE_UNUSED(_id);
		return 0;
	}

	virtual bool cacheRead(u64 _id, void *_data, u32 _size) override
	{
		CE_UNUSED_3(_id, _data, _size);
		return false;
	}

	virtual void cacheWrite(u64 _id, const void *_data, u32 _size) override
	{
		CE_UNUSED_3(_id, _data, _size);
	}

	virtual void screenShot(const char *_filePath, u32 _width, u32 _height, u32 _pitch, const void *_data, u32 _size, bool _yflip) override
	{
		CE_UNUSED(_size);

		bx::Error err;
		bx::FileWriter writer;
		if (bx::open(&writer, _filePath, false, &err)) {
			bimg::imageWritePng(&writer
				, _width
				, _height
				, _pitch
				, _data
				, bimg::TextureFormat::BGRA8
				, _yflip
				, &err
				);
			bx::close(&writer);
		}

		_screenshot_path = _filePath;
		_screenshot_ready = 1;
		++device()->_needs_draw; // 1 frame for _screenshot_ready to be evaluated.
	}

	virtual void captureBegin(u32 _width, u32 _height, u32 _pitch, bgfx::TextureFormat::Enum _format, bool _yflip) override
	{
		CE_UNUSED_5(_width, _height, _pitch, _format, _yflip);
	}

	virtual void captureEnd() override
	{
	}

	virtual void captureFrame(const void *_data, u32 _size) override
	{
		CE_UNUSED_2(_data, _size);
	}
};

struct BgfxAllocator : public bx::AllocatorI
{
	ProxyAllocator _allocator;

	explicit BgfxAllocator(Allocator &a)
		: _allocator(a, "bgfx")
	{
	}

	~BgfxAllocator()
	{
	}

	virtual void *realloc(void *_ptr, size_t _size, size_t _align, const char * /*_file*/, u32 /*_line*/)
	{
		return _allocator.reallocate(_ptr, _size, _align);
	}
};

static void device_command_pause(ConsoleServer & /*cs*/, u32 /*client_id*/, const JsonArray & /*args*/, void * /*user_data*/)
{
	device()->pause();
}

static void device_command_unpause(ConsoleServer & /*cs*/, u32 /*client_id*/, const JsonArray & /*args*/, void * /*user_data*/)
{
	device()->unpause();
}

static void device_command_game(ConsoleServer &cs, u32 client_id, const JsonArray &args, void *user_data)
{
	CE_UNUSED(user_data);
	TempAllocator1024 ta;

	if (array::size(args) < 2) {
		cs.error(client_id, "Usage: game <pause|resume>");
		return;
	}

	DynamicString subcmd(ta);
	sjson::parse_string(subcmd, args[1]);
	if (subcmd == "pause") {
		device()->pause();
	} else if (subcmd == "resume") {
		device()->unpause();
	} else {
		loge(DEVICE, "Unknown game parameter");
	}
}

static void device_command_crash(ConsoleServer &cs, u32 client_id, const JsonArray &args, void *user_data)
{
	CE_UNUSED(user_data);
	TempAllocator1024 ta;

	struct
	{
		const char *type_name;
		const char *desc;
		CrashType::Enum type;
	}
	crash_info[] =
	{
		{ "div_by_zero", "Divide a number by zero.",       CrashType::DIVISION_BY_ZERO   },
		{ "unaligned",   "Do an unaligned memory access.", CrashType::UNALIGNED_ACCESS   },
		{ "segfault",    "Trigger a segmentation fault.",  CrashType::SEGMENTATION_FAULT },
		{ "oom",         "Allocate too much memory.",      CrashType::OUT_OF_MEMORY      },
		{ "assert",      "Call CE_ASSERT(false).",         CrashType::ASSERT             }
	};
	CE_STATIC_ASSERT(countof(crash_info) == CrashType::COUNT);

	if (array::size(args) < 2) {
		cs.error(client_id, "Usage: crash <type>");
		return;
	}

	DynamicString subcmd(ta);
	sjson::parse_string(subcmd, args[1]);

	if (subcmd == "help") {
		for (u32 i = 0; i < countof(crash_info); ++i) {
			logi(DEVICE, "%s %s", crash_info[i].type_name, crash_info[i].desc);
		}
	} else {
		// Decode crash type.
		CrashType::Enum crash_type = CrashType::COUNT;
		for (u32 i = 0; i < countof(crash_info); ++i) {
			if (subcmd == crash_info[i].type_name) {
				crash_type = crash_info[i].type;
				break;
			}
		}

		if (crash_type == CrashType::COUNT)
			loge(DEVICE, "Unknown crash parameter");
		else
			debug::crash(crash_type);
	}
}

static void device_message_resize(ConsoleServer & /*cs*/, u32 /*client_id*/, const char *json, void * /*user_data*/)
{
	TempAllocator256 ta;
	JsonObject obj(ta);
	s32 width;
	s32 height;

	sjson::parse(obj, json);
	width = sjson::parse_int(obj["width"]);
	height = sjson::parse_int(obj["height"]);

	device()->_window->resize((u16)width, (u16)height);
}

static void device_message_frame(ConsoleServer & /*cs*/, u32 /*client_id*/, const char * /*json*/, void *user_data)
{
	++((Device *)user_data)->_needs_draw;
}

static void device_message_quit(ConsoleServer &cs, u32 client_id, const char *json, void *user_data)
{
	CE_UNUSED_3(cs, client_id, json);
	((Device *)user_data)->quit();
}

static void device_message_refresh(ConsoleServer &cs, u32 client_id, const char *json, void *user_data)
{
	CE_UNUSED_2(cs, client_id);
	((Device *)user_data)->refresh(json);

	TempAllocator512 ta;
	StringStream ss(ta);
	ss << "{";
	ss << "\"type\":\"refresh\",";
	ss << "\"success\":" << (true ? "true" : "false");
	ss << "}";

	cs.send(client_id, string_stream::c_str(ss));
}

Device::Device(const DeviceOptions &opts, ConsoleServer &cs)
	: _allocator(default_allocator(), CROWN_MAX_SUBSYSTEMS_HEAP)
	, _options(opts)
	, _boot_config(default_allocator())
	, _console_server(&cs)
	, _data_filesystem(NULL)
	, _resource_loader(NULL)
	, _resource_manager(NULL)
	, _bgfx_allocator(NULL)
	, _bgfx_callback(NULL)
	, _shader_manager(NULL)
	, _material_manager(NULL)
	, _input_manager(NULL)
	, _unit_manager(NULL)
	, _lua_environment(NULL)
	, _pipeline(NULL)
	, _display(NULL)
	, _window(NULL)
	, _timestep_policy(TimestepPolicy::VARIABLE)
	, _render_config_resource(NULL)
	, _width(CROWN_DEFAULT_WINDOW_WIDTH)
	, _height(CROWN_DEFAULT_WINDOW_HEIGHT)
	, _exit_code(EXIT_SUCCESS)
	, _quit(0)
	, _paused(0)
	, _last_paused(0)
	, _needs_draw(1)
{
	list::init_head(_worlds);
}

bool Device::process_events()
{
	bool exit = false;

	OsEvent event;
	while (next_event(event)) {
		switch (event.type) {
		case OsEventType::BUTTON:
		case OsEventType::AXIS:
		case OsEventType::STATUS:
			_input_manager->read(event);
			break;

		case OsEventType::RESOLUTION:
			_width  = event.resolution.width;
			_height = event.resolution.height;
			break;

		case OsEventType::EXIT:
			exit = true;
			break;

		case OsEventType::PAUSE:
			pause();
			break;

		case OsEventType::RESUME:
			unpause();
			break;

		case OsEventType::TEXT:
			break;

		default:
			CE_FATAL("Unknown OS event");
			break;
		}
	}

	return exit;
}

void Device::set_timestep_policy(TimestepPolicy::Enum tp)
{
	if (_timestep_policy == tp)
		return;

	_timestep_policy = tp;
}

void Device::set_timestep_smoothing(u32 num_samples, u32 num_outliers, f32 average_cap)
{
	_delta_time_filter.set_smoothing(num_samples, num_outliers, average_cap);
}

bool Device::frame()
{
	if (CE_UNLIKELY(process_events() || _quit != 0))
		return true;

	if (CE_UNLIKELY(_paused == 0 && _last_paused == 1)) {
		_last_paused = 0;
		logi(DEVICE, "Resumed");
	} else if (CE_UNLIKELY(_paused == 1 && _last_paused == 0)) {
		_last_paused = 1;
		logi(DEVICE, "Paused");

		// Having the cursor hidden while the game is paused
		// is confusing and should be avoided.
		if (_window)
			_window->set_cursor_mode(CursorMode::NORMAL);
	}

	const s64 time = time::now();
	const s64 raw_dt_ticks = time - _last_time;
	const f32 raw_dt = f32(time::seconds(raw_dt_ticks));
	_last_time = time;

	profiler_globals::clear();
	RECORD_FLOAT("device.dt", raw_dt);
	RECORD_FLOAT("device.fps", 1.0f/raw_dt);

	f32 dt;
	if (_timestep_policy == TimestepPolicy::SMOOTHED) {
		f32 smoothed_dt = _delta_time_filter.filter(raw_dt_ticks);
		RECORD_FLOAT("device.smoothed_dt", smoothed_dt);
		RECORD_FLOAT("device.smoothed_fps", 1.0f/smoothed_dt);
		dt = smoothed_dt;
	} else {
		dt = raw_dt;
	}

	if (CE_UNLIKELY(_width != _prev_width || _height != _prev_height)) {
		_prev_width = _width;
		_prev_height = _height;
		bgfx::reset(_width, _height, (_boot_config.vsync ? BGFX_RESET_VSYNC : BGFX_RESET_NONE));
		_pipeline->reset(_width, _height);

		// Force pipeline reset in one cycle.
		bgfx::frame();
		bgfx::frame();

		// Force redraw.
		++_needs_draw;
	}

	// Only block if redraw is not needed.
	const bool sync = !_needs_draw;
#if !CROWN_PLATFORM_EMSCRIPTEN
	_console_server->execute_message_handlers(sync);
#endif

	if (CE_UNLIKELY(!_needs_draw))
		return false;

	if (CE_LIKELY(_paused == 0)) {
		_resource_manager->complete_requests();

		{
			const s64 t0 = time::now();
			ArgType::Enum arg_types = ArgType::FLOAT;
			Arg args; args.float_value = dt;
			_lua_environment->call_global("update", &arg_types, &args, 1);
			RECORD_FLOAT("lua.update", f32(time::seconds(time::now() - t0)));
		}
		{
			const s64 t0 = time::now();
			ArgType::Enum arg_types = ArgType::FLOAT;
			Arg args; args.float_value = dt;
			_lua_environment->call_global("render", &arg_types, &args, 1);
			RECORD_FLOAT("lua.render", f32(time::seconds(time::now() - t0)));
		}

		if (_bgfx_callback->_screenshot_ready) {
			_bgfx_callback->_screenshot_ready = 0;
			ArgType::Enum arg_types = ArgType::STRING;
			Arg args; args.string_value = _bgfx_callback->_screenshot_path.c_str();
			_lua_environment->call_global("screenshot", &arg_types, &args, 1);
		}
	}

	_lua_environment->reset_temporaries();
	_input_manager->update();

	const bgfx::Stats *stats = bgfx::getStats();
	RECORD_FLOAT("bgfx.gpu_time", f32(f64(stats->gpuTimeEnd - stats->gpuTimeBegin)/stats->gpuTimerFreq));
	RECORD_FLOAT("bgfx.cpu_time", f32(f64(stats->cpuTimeEnd - stats->cpuTimeBegin)/stats->cpuTimerFreq));

	profiler_globals::flush();

	graph_globals::draw_all(_width, _height);

	bgfx::frame();

	if (_needs_draw-- == 1)
		_needs_draw = (int)!_options._pumped;

	return false;
}

int Device::main_loop()
{
	s64 run_t0 = time::now();

	_console_server->register_command_name("pause",   "Pause the engine.",  device_command_pause,   this);
	_console_server->register_command_name("unpause", "Resume the engine.", device_command_unpause, this);
	_console_server->register_command_name("game",    "Pause/resume the engine.", device_command_game, this);
	_console_server->register_command_name("crash",   "Crash the engine.", device_command_crash, this);

	_console_server->register_message_type("resize",  device_message_resize,  this);
	_console_server->register_message_type("frame",   device_message_frame,   this);
	_console_server->register_message_type("quit",    device_message_quit,    this);
	_console_server->register_message_type("refresh", device_message_refresh, this);

#if !CROWN_PLATFORM_EMSCRIPTEN
	_console_server->listen(_options._console_port, _options._wait_console);
#endif

	bool is_bundle = true;
#if CROWN_PLATFORM_ANDROID
	_data_filesystem = CE_NEW(_allocator, FilesystemApk)(default_allocator(), const_cast<AAssetManager *>((AAssetManager *)_options._asset_manager));
#else
	_data_filesystem = CE_NEW(_allocator, FilesystemDisk)(default_allocator());
	{
		char cwd[1024];
		const char *data_dir = NULL;

		if (_options._bundle_dir.empty() && _options._data_dir.empty()) {
			data_dir = os::getcwd(cwd, sizeof(cwd));
		} else {
			if (!_options._bundle_dir.empty()) {
				data_dir = _options._bundle_dir.c_str();
			} else {
				is_bundle = false;
				if (!_options._data_dir.empty())
					data_dir = _options._data_dir.c_str();
				else
					data_dir = os::getcwd(cwd, sizeof(cwd));
			}
		}

		((FilesystemDisk *)_data_filesystem)->set_prefix(data_dir);
	}
#endif // if CROWN_PLATFORM_ANDROID

	logi(DEVICE, "Crown %s %s %s", CROWN_VERSION, CROWN_PLATFORM_NAME, CROWN_ARCH_NAME);

	_shader_manager   = CE_NEW(_allocator, ShaderManager)(default_allocator());

	_resource_loader  = CE_NEW(_allocator, ResourceLoader)(*_data_filesystem, is_bundle);
	_resource_loader->register_fallback(RESOURCE_TYPE_TEXTURE,  STRING_ID_64("core/fallback/fallback", 0xd09058ae71962248));
	_resource_loader->register_fallback(RESOURCE_TYPE_MATERIAL, STRING_ID_64("core/fallback/fallback", 0xd09058ae71962248));
	_resource_loader->register_fallback(RESOURCE_TYPE_UNIT,     STRING_ID_64("core/fallback/fallback", 0xd09058ae71962248));

	_resource_manager = CE_NEW(_allocator, ResourceManager)(*_resource_loader);
	_resource_manager->register_type(RESOURCE_TYPE_CONFIG,           RESOURCE_VERSION_CONFIG,           config_resource_internal::load,  config_resource_internal::unload,  NULL,                               NULL);
	_resource_manager->register_type(RESOURCE_TYPE_FONT,             RESOURCE_VERSION_FONT,             NULL,                            NULL,                              NULL,                               NULL);
	_resource_manager->register_type(RESOURCE_TYPE_LEVEL,            RESOURCE_VERSION_LEVEL,            NULL,                            NULL,                              NULL,                               NULL);
	_resource_manager->register_type(RESOURCE_TYPE_MATERIAL,         RESOURCE_VERSION_MATERIAL,         NULL,                            NULL,                              material_resource_internal::online, material_resource_internal::offline);
	_resource_manager->register_type(RESOURCE_TYPE_MESH,             RESOURCE_VERSION_MESH,             mesh_resource_internal::load,    mesh_resource_internal::unload,    mesh_resource_internal::online,     mesh_resource_internal::offline);
	_resource_manager->register_type(RESOURCE_TYPE_MESH_SKELETON,    RESOURCE_VERSION_MESH_SKELETON,    NULL,                            NULL,                              NULL,                               NULL);
	_resource_manager->register_type(RESOURCE_TYPE_MESH_ANIMATION,   RESOURCE_VERSION_MESH_ANIMATION,   NULL,                            NULL,                              NULL,                               NULL);
	_resource_manager->register_type(RESOURCE_TYPE_PACKAGE,          RESOURCE_VERSION_PACKAGE,          NULL,                            NULL,                              NULL,                               NULL);
	_resource_manager->register_type(RESOURCE_TYPE_PHYSICS_CONFIG,   RESOURCE_VERSION_PHYSICS_CONFIG,   NULL,                            NULL,                              NULL,                               NULL);
	_resource_manager->register_type(RESOURCE_TYPE_RENDER_CONFIG,    RESOURCE_VERSION_RENDER_CONFIG,    NULL,                            NULL,                              NULL,                               NULL);
	_resource_manager->register_type(RESOURCE_TYPE_SCRIPT,           RESOURCE_VERSION_SCRIPT,           NULL,                            NULL,                              NULL,                               NULL);
	_resource_manager->register_type(RESOURCE_TYPE_SHADER,           RESOURCE_VERSION_SHADER,           shader_resource_internal::load,  shader_resource_internal::unload,  shader_resource_internal::online,   shader_resource_internal::offline);
	_resource_manager->register_type(RESOURCE_TYPE_SOUND,            RESOURCE_VERSION_SOUND,            NULL,                            NULL,                              NULL,                               NULL);
	_resource_manager->register_type(RESOURCE_TYPE_SPRITE,           RESOURCE_VERSION_SPRITE,           NULL,                            NULL,                              NULL,                               NULL);
	_resource_manager->register_type(RESOURCE_TYPE_SPRITE_ANIMATION, RESOURCE_VERSION_SPRITE_ANIMATION, NULL,                            NULL,                              NULL,                               NULL);
	_resource_manager->register_type(RESOURCE_TYPE_STATE_MACHINE,    RESOURCE_VERSION_STATE_MACHINE,    NULL,                            NULL,                              NULL,                               NULL);
	_resource_manager->register_type(RESOURCE_TYPE_TEXTURE,          RESOURCE_VERSION_TEXTURE,          texture_resource_internal::load, texture_resource_internal::unload, texture_resource_internal::online,  texture_resource_internal::offline);
	_resource_manager->register_type(RESOURCE_TYPE_UNIT,             RESOURCE_VERSION_UNIT,             NULL,                            NULL,                              NULL,                               NULL);

	_material_manager = CE_NEW(_allocator, MaterialManager)(default_allocator(), *_resource_manager, *_shader_manager);

	// Read config
	{
		TempAllocator512 ta;
		DynamicString boot_dir(ta);
		if (_options._boot_dir != NULL) {
			boot_dir += _options._boot_dir;
			boot_dir += '/';
		}
		boot_dir += CROWN_BOOT_CONFIG;

		const StringId64 config_name(boot_dir.c_str());

		while (!_resource_manager->try_load(PACKAGE_RESOURCE_NONE, RESOURCE_TYPE_CONFIG, config_name, 0)) {
			_resource_manager->complete_requests();
#if CROWN_PLATFORM_EMSCRIPTEN
			os::sleep(16);
#endif
		}
		while (!_resource_manager->can_get(RESOURCE_TYPE_CONFIG, config_name)) {
			_resource_manager->complete_requests();
#if CROWN_PLATFORM_EMSCRIPTEN
			os::sleep(16);
#endif
		}

		_boot_config.parse((char *)_resource_manager->get(RESOURCE_TYPE_CONFIG, config_name));
		_resource_manager->unload(RESOURCE_TYPE_CONFIG, config_name);
	}

	// Init all remaining subsystems
	_display = display::create(_allocator);

#if !CROWN_PLATFORM_EMSCRIPTEN
	if (_options._window_width.has_changed() || _options._window_height.has_changed()) {
		_width  = _options._window_width.value();
		_height = _options._window_height.value();
	} else {
		_width  = _boot_config.window_w;
		_height = _boot_config.window_h;
	}
#endif

	_window = window::create(_allocator);
	_window->open(_options._window_x
		, _options._window_y
		, _width
		, _height
		, _options._parent_window
		);
	_window->set_title(_boot_config.window_title.c_str());
	_window->set_fullscreen(_boot_config.fullscreen);

	if (!_options._hidden)
		_window->show();

	_bgfx_allocator = CE_NEW(_allocator, BgfxAllocator)(default_allocator());
	_bgfx_callback  = CE_NEW(_allocator, BgfxCallback)(default_allocator());

	bgfx::Init init;
	init.resolution.width  = _width;
	init.resolution.height = _height;
	init.resolution.reset  = _boot_config.vsync ? BGFX_RESET_VSYNC : BGFX_RESET_NONE;
	init.callback  = _bgfx_callback;
	init.allocator = _bgfx_allocator;
	init.platformData.ndt = _window->native_display();
	init.platformData.nwh = _window->native_handle();
	init.vendorId = BGFX_PCI_ID_NONE;
#if CROWN_PLATFORM_ANDROID || CROWN_PLATFORM_EMSCRIPTEN
	init.type = bgfx::RendererType::OpenGLES;
#elif CROWN_PLATFORM_LINUX
	init.type = bgfx::RendererType::OpenGL;
#elif CROWN_PLATFORM_WINDOWS
	init.type = bgfx::RendererType::Direct3D11;
#else
	#error "Unknown platform"
#endif
	bgfx::init(init);

	_input_manager    = CE_NEW(_allocator, InputManager)(default_allocator());
	_unit_manager     = CE_NEW(_allocator, UnitManager)(default_allocator());
	_lua_environment  = CE_NEW(_allocator, LuaEnvironment)();
	_lua_environment->register_console_commands(*_console_server);

	sound_world::init();

	// Load boot package.
	ResourcePackage *boot_package = create_resource_package(_boot_config.boot_package_name);
	boot_package->load();
	boot_package->flush();

	// Load render config package.
	ResourcePackage *render_config_package = create_resource_package(_boot_config.render_config_name);
	render_config_package->load();
	render_config_package->flush();

	_render_config_resource = (RenderConfigResource *)_resource_manager->get(RESOURCE_TYPE_RENDER_CONFIG, _boot_config.render_config_name);

	physics_globals::init(_allocator, default_allocator(), &_boot_config.physics_settings);

	_lua_environment->load_libs();
	_lua_environment->require(_boot_config.boot_script_name.c_str());
	_lua_environment->execute_string(_options._lua_string.c_str());

	_pipeline = CE_NEW(_allocator, Pipeline)(*_shader_manager);
	_pipeline->create(_width, _height, _render_config_resource->render_settings);

	graph_globals::init(_allocator, *_pipeline, *_console_server);

	logi(DEVICE, "Initialized in " TIME_FMT, time::seconds(time::now() - run_t0));

	_lua_environment->call_global("init");

	_prev_width = _width;
	_prev_height = _height;
	_last_time = time::now();

#if CROWN_PLATFORM_EMSCRIPTEN
	emscripten_set_main_loop_arg([](void *thiz) { ((Device *)thiz)->frame(); }, this, 0, -1);
#else
	while (!frame()) { }
#endif

	_lua_environment->call_global("shutdown");

	render_config_package->unload();
	destroy_resource_package(*render_config_package);

	boot_package->unload();
	destroy_resource_package(*boot_package);

	physics_globals::shutdown(_allocator, default_allocator());
	sound_world::shutdown();
	graph_globals::shutdown();

	_pipeline->destroy();
	CE_DELETE(_allocator, _pipeline);
	CE_DELETE(_allocator, _lua_environment);
	CE_DELETE(_allocator, _unit_manager);
	CE_DELETE(_allocator, _input_manager);
	CE_DELETE(_allocator, _resource_manager);
	CE_DELETE(_allocator, _resource_loader);
	CE_DELETE(_allocator, _material_manager);
	CE_DELETE(_allocator, _shader_manager);

	bgfx::shutdown();
	CE_DELETE(_allocator, _bgfx_callback);
	CE_DELETE(_allocator, _bgfx_allocator);

	_window->close();
	window::destroy(_allocator, *_window);
	display::destroy(_allocator, *_display);

	CE_DELETE(_allocator, _data_filesystem);

	_allocator.clear();
	return _exit_code;
}

void Device::quit(int exit_code)
{
	_exit_code = exit_code & 0xff;
	_quit = 1;
}

int Device::argc() const
{
	return _options._argc;
}

const char **Device::argv() const
{
	return (const char **)_options._argv;
}

void Device::pause()
{
	_paused = 1;
}

void Device::unpause()
{
	_paused = 0;
}

void Device::resolution(u16 &width, u16 &height)
{
	width = _width;
	height = _height;
}

void Device::render(World &world, UnitId camera_unit)
{
	CameraInstance camera = world.camera_instance(camera_unit);

	const f32 aspect_ratio = (_boot_config.aspect_ratio == -1.0f
		? (f32)_width/(f32)_height
		: _boot_config.aspect_ratio
		);
	const Matrix4x4 view = world.camera_view_matrix(camera);
	const Matrix4x4 proj = world.camera_projection_matrix(camera, aspect_ratio);
	const Matrix4x4 persp = world.camera_projection_matrix(camera, aspect_ratio, ProjectionType::PERSPECTIVE);

	world.render(view, proj, persp);
	_pipeline->render(_width, _height, view, proj);
}

World *Device::create_world()
{
	World *world = CE_NEW(default_allocator(), World)(default_allocator()
		, *_resource_manager
		, *_shader_manager
		, *_material_manager
		, *_unit_manager
		, *_lua_environment
		, *_pipeline
		);

	list::add(world->_node, _worlds);
	return world;
}

void Device::destroy_world(World &world)
{
	list::remove(world._node);
	CE_DELETE(default_allocator(), &world);
}

ResourcePackage *Device::create_resource_package(StringId64 id)
{
	return CE_NEW(default_allocator(), ResourcePackage)(id, *_resource_manager);
}

void Device::destroy_resource_package(ResourcePackage &rp)
{
	CE_DELETE(default_allocator(), &rp);
}

void Device::refresh(const char *json)
{
#if CROWN_CAN_RELOAD
	TempAllocator4096 ta;
	JsonObject obj(ta);
	JsonArray list(ta);
	DynamicString type(ta);
	sjson::parse(obj, json);

	bool refresh_lua = false;
	sjson::parse_array(list, obj["list"]);
	for (u32 i = 0; i < array::size(list); ++i) {
		DynamicString resource(ta);
		sjson::parse_string(resource, list[i]);

		const char *type = resource_type(resource.c_str());
		const u32 len = resource_name_length(type, resource.c_str());

		StringId64 resource_type(type);
		StringId64 resource_name(resource.c_str(), len);
		bool is_type_reloadable = resource_type == RESOURCE_TYPE_SCRIPT
			|| resource_type == RESOURCE_TYPE_TEXTURE
			|| resource_type == RESOURCE_TYPE_SHADER
			|| resource_type == RESOURCE_TYPE_MATERIAL
			|| resource_type == RESOURCE_TYPE_RENDER_CONFIG
			|| resource_type == RESOURCE_TYPE_UNIT
			|| resource_type == RESOURCE_TYPE_STATE_MACHINE
			;

		if (is_type_reloadable && _resource_manager->can_get(resource_type, resource_name)) {
			const void *old_resource = _resource_manager->get(resource_type, resource_name);
			const void *new_resource = _resource_manager->reload(resource_type, resource_name);

			if (resource_type == RESOURCE_TYPE_SCRIPT) {
				refresh_lua = true;
			} else if (resource_type == RESOURCE_TYPE_TEXTURE) {
				_material_manager->reload_textures((TextureResource *)old_resource, (TextureResource *)new_resource);
			} else if (resource_type == RESOURCE_TYPE_SHADER) {
				_pipeline->reload_shaders((ShaderResource *)old_resource, (ShaderResource *)new_resource);
				_material_manager->reload_shaders((ShaderResource *)old_resource, (ShaderResource *)new_resource);
			} else if (resource_type == RESOURCE_TYPE_MATERIAL) {
				ListNode *cur;
				list_for_each(cur, &_worlds)
				{
					World *w = (World *)container_of(cur, World, _node);
					w->reload_materials((MaterialResource *)old_resource, (MaterialResource *)new_resource);
				}
			} else if (resource_type == RESOURCE_TYPE_RENDER_CONFIG) {
				if (_render_config_resource == old_resource) {
					_render_config_resource = (RenderConfigResource *)new_resource;
					_pipeline->destroy();
					_pipeline->create(_width, _height, _render_config_resource->render_settings);
				}
			} else if (resource_type == RESOURCE_TYPE_UNIT) {
				ListNode *cur;
				list_for_each(cur, &_worlds)
				{
					World *w = (World *)container_of(cur, World, _node);
					w->reload_units((UnitResource *)old_resource, (UnitResource *)new_resource);
				}
			} else if (resource_type == RESOURCE_TYPE_STATE_MACHINE) {
				ListNode *cur;
				list_for_each(cur, &_worlds)
				{
					World *w = (World *)container_of(cur, World, _node);
					w->_animation_state_machine->reload((const StateMachineResource *)old_resource, (const StateMachineResource *)new_resource);
				}
			}
		}
	}

	if (array::size(list)) {
		if (refresh_lua)
			_lua_environment->reload();
	}

	if (_paused)
		unpause();
#else
	CE_UNUSED(json);
#endif // if CROWN_CAN_RELOAD
}

void Device::screenshot(const char *path)
{
	bgfx::requestScreenShot(BGFX_INVALID_HANDLE, path);
	++device()->_needs_draw; // 1 frame for the request to be fulfilled.
}

Device *_device = NULL;

int main_runtime(const DeviceOptions &opts)
{
	CE_ASSERT(_device == NULL, "Crown already initialized");
	console_server_globals::init();
	_device = CE_NEW(default_allocator(), Device)(opts, *console_server());
	int ec = _device->main_loop();
	CE_DELETE(default_allocator(), _device);
	_device = NULL;
	console_server_globals::shutdown();
	return ec;
}

Device *device()
{
	return crown::_device;
}

} // namespace crown
