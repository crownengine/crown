/*
 * Copyright (c) 2012-2016 Daniele Bartolini and individual contributors.
 * License: https://github.com/taylor001/crown/blob/master/LICENSE
 */

#include "apk_filesystem.h"
#include "array.h"
#include "audio.h"
#include "bundle_compiler.h"
#include "config.h"
#include "config_resource.h"
#include "console_server.h"
#include "device.h"
#include "disk_filesystem.h"
#include "file.h"
#include "filesystem.h"
#include "font_resource.h"
#include "input_device.h"
#include "input_manager.h"
#include "level_resource.h"
#include "log.h"
#include "lua_environment.h"
#include "lua_resource.h"
#include "map.h"
#include "material_manager.h"
#include "material_resource.h"
#include "matrix4x4.h"
#include "memory.h"
#include "mesh_resource.h"
#include "os.h"
#include "os_event_queue.h"
#include "package_resource.h"
#include "path.h"
#include "physics.h"
#include "physics_resource.h"
#include "profiler.h"
#include "proxy_allocator.h"
#include "resource_loader.h"
#include "resource_manager.h"
#include "resource_package.h"
#include "shader_manager.h"
#include "shader_resource.h"
#include "sjson.h"
#include "sound_resource.h"
#include "sprite_resource.h"
#include "string_stream.h"
#include "string_utils.h"
#include "temp_allocator.h"
#include "texture_resource.h"
#include "types.h"
#include "unit_manager.h"
#include "unit_resource.h"
#include "vector3.h"
#include "world.h"
#include <bgfx/bgfx.h>
#include <bx/allocator.h>

#define MAX_SUBSYSTEMS_HEAP 8 * 1024 * 1024

namespace crown
{
extern bool next_event(OsEvent& ev);

struct BgfxCallback : public bgfx::CallbackI
{
	virtual void fatal(bgfx::Fatal::Enum _code, const char* _str)
	{
		CE_ASSERT(false, "Fatal error: 0x%08x: %s", _code, _str);
		CE_UNUSED(_code);
		CE_UNUSED(_str);
	}

	virtual void traceVargs(const char* /*_filePath*/, u16 /*_line*/, const char* _format, va_list _argList)
	{
		char buf[2048];
		strncpy(buf, _format, sizeof(buf));
		buf[strlen32(buf)-1] = '\0'; // Remove trailing newline
		CE_LOGDV(buf, _argList);
	}

	virtual u32 cacheReadSize(u64 /*_id*/)
	{
		return 0;
	}

	virtual bool cacheRead(u64 /*_id*/, void* /*_data*/, u32 /*_size*/)
	{
		return false;
	}

	virtual void cacheWrite(u64 /*_id*/, const void* /*_data*/, u32 /*_size*/)
	{
	}

	virtual void screenShot(const char* /*_filePath*/, u32 /*_width*/, u32 /*_height*/, u32 /*_pitch*/, const void* /*_data*/, u32 /*_size*/, bool /*_yflip*/)
	{
	}

	virtual void captureBegin(u32 /*_width*/, u32 /*_height*/, u32 /*_pitch*/, bgfx::TextureFormat::Enum /*_format*/, bool /*_yflip*/)
	{
	}

	virtual void captureEnd()
	{
	}

	virtual void captureFrame(const void* /*_data*/, u32 /*_size*/)
	{
	}
};

struct BgfxAllocator : public bx::AllocatorI
{
	BgfxAllocator(Allocator& a)
		: _allocator(a, "bgfx")
	{
	}

	virtual void* realloc(void* _ptr, size_t _size, size_t _align, const char* /*_file*/, u32 /*_line*/)
	{
		if (!_ptr)
			return _allocator.allocate((u32)_size, (u32)_align == 0 ? 1 : (u32)_align);

		if (_size == 0)
		{
			_allocator.deallocate(_ptr);
			return NULL;
		}

		// Realloc
		void* p = _allocator.allocate((u32)_size, (u32)_align == 0 ? 1 : (u32)_align);
		_allocator.deallocate(_ptr);
		return p;
	}

private:

	ProxyAllocator _allocator;
};

static void console_command_script(void* /*data*/, ConsoleServer& /*cs*/, TCPSocket /*client*/, const char* json)
{
	TempAllocator4096 ta;
	JsonObject obj(ta);
	DynamicString script(ta);
	sjson::parse(json, obj);
	sjson::parse_string(obj["script"], script);
	device()->lua_environment()->execute_string(script.c_str());
}

static void console_command_reload(void* /*data*/, ConsoleServer& /*cs*/, TCPSocket /*client*/, const char* json)
{
	TempAllocator4096 ta;
	JsonObject obj(ta);
	sjson::parse(json, obj);
	StringId64 type = sjson::parse_resource_id(obj["resource_type"]);
	StringId64 name = sjson::parse_resource_id(obj["resource_name"]);
 	device()->reload(type, name);
}

static void console_command_pause(void* /*data*/, ConsoleServer& /*cs*/, TCPSocket /*client*/, const char* /*json*/)
{
	device()->pause();
}

static void console_command_unpause(void* /*data*/, ConsoleServer& /*cs*/, TCPSocket /*client*/, const char* /*json*/)
{
	device()->unpause();
}

static void console_command_compile(void* data, ConsoleServer& cs, TCPSocket client, const char* json)
{
	TempAllocator4096 ta;
	JsonObject obj(ta);
	sjson::parse(json, obj);

	DynamicString id(ta);
	DynamicString bundle_dir(ta);
	DynamicString platform(ta);
	sjson::parse_string(obj["id"], id);
	sjson::parse_string(obj["bundle_dir"], bundle_dir);
	sjson::parse_string(obj["platform"], platform);

	{
		TempAllocator512 ta;
		StringStream ss(ta);
		ss << "{\"type\":\"compile\",\"id\":\"" << id.c_str() << "\",\"start\":true}";
		cs.send(client, string_stream::c_str(ss));
	}

	BundleCompiler* bc = (BundleCompiler*)data;
	bool succ = bc->compile(bundle_dir.c_str(), platform.c_str());

	{
		TempAllocator512 ta;
		StringStream ss(ta);
		ss << "{\"type\":\"compile\",\"id\":\"" << id.c_str() << "\",\"success\":" << (succ ? "true" : "false") << "}";
		cs.send(client, string_stream::c_str(ss));
	}
}

Device::Device(const DeviceOptions& opts)
	: _allocator(default_allocator(), MAX_SUBSYSTEMS_HEAP)
	, _device_options(opts)
	, _console_server(NULL)
	, _bundle_compiler(NULL)
	, _bundle_filesystem(NULL)
	, _last_log(NULL)
	, _resource_loader(NULL)
	, _resource_manager(NULL)
	, _bgfx_allocator(NULL)
	, _bgfx_callback(NULL)
	, _shader_manager(NULL)
	, _material_manager(NULL)
	, _input_manager(NULL)
	, _unit_manager(NULL)
	, _lua_environment(NULL)
	, _display(NULL)
	, _window(NULL)
	, _boot_package_name(u64(0))
	, _boot_script_name(u64(0))
	, _boot_package(NULL)
	, _config_window_x(0)
	, _config_window_y(0)
	, _config_window_w(CROWN_DEFAULT_WINDOW_WIDTH)
	, _config_window_h(CROWN_DEFAULT_WINDOW_HEIGHT)
	, _worlds(default_allocator())
	, _width(0)
	, _height(0)
	, _mouse_curr_x(0)
	, _mouse_curr_y(0)
	, _mouse_last_x(0)
	, _mouse_last_y(0)
	, _quit(false)
	, _paused(false)
	, _frame_count(0)
	, _last_time(0)
	, _current_time(0)
	, _last_delta_time(0.0f)
	, _time_since_start(0.0)
{
}

void Device::read_config()
{
	TempAllocator4096 ta;
	DynamicString boot_dir(ta);

	if (_device_options._boot_dir != NULL)
	{
		boot_dir += _device_options._boot_dir;
		boot_dir += '/';
	}

	boot_dir += CROWN_BOOT_CONFIG;

	const StringId64 config_name(boot_dir.c_str());

	_resource_manager->load(RESOURCE_TYPE_CONFIG, config_name);
	_resource_manager->flush();
	const char* cfile = (const char*)_resource_manager->get(RESOURCE_TYPE_CONFIG, config_name);

	JsonObject config(ta);
	sjson::parse(cfile, config);

	_boot_script_name  = sjson::parse_resource_id(config["boot_script"]);
	_boot_package_name = sjson::parse_resource_id(config["boot_package"]);

	// Platform-specific configs
	if (map::has(config, FixedString(CROWN_PLATFORM_NAME)))
	{
		JsonObject platform(ta);
		sjson::parse(config[CROWN_PLATFORM_NAME], platform);

		if (map::has(platform, FixedString("window_width")))
		{
			_config_window_w = (u16)sjson::parse_int(platform["window_width"]);
		}
		if (map::has(platform, FixedString("window_height")))
		{
			_config_window_h = (u16)sjson::parse_int(platform["window_height"]);
		}
	}

	_resource_manager->unload(RESOURCE_TYPE_CONFIG, config_name);
}

bool Device::process_events()
{
	OsEvent event;
	bool exit = false;
	InputManager* im = _input_manager;

	const s16 dt_x = _mouse_curr_x - _mouse_last_x;
	const s16 dt_y = _mouse_curr_y - _mouse_last_y;
	im->mouse()->set_axis(MouseAxis::CURSOR_DELTA, vector3(dt_x, dt_y, 0.0f));
	_mouse_last_x = _mouse_curr_x;
	_mouse_last_y = _mouse_curr_y;

	while(next_event(event))
	{
		if (event.type == OsEvent::NONE) continue;

		switch (event.type)
		{
			case OsEvent::TOUCH:
			{
				const OsTouchEvent& ev = event.touch;
				switch (ev.type)
				{
					case OsTouchEvent::POINTER:
						im->touch()->set_button_state(ev.pointer_id, ev.pressed);
						break;
					case OsTouchEvent::MOVE:
						im->touch()->set_axis(ev.pointer_id, vector3(ev.x, ev.y, 0.0f));
						break;
					default:
						CE_FATAL("Unknown touch event type");
						break;
				}
				break;
			}
			case OsEvent::MOUSE:
			{
				const OsMouseEvent& ev = event.mouse;
				switch (ev.type)
				{
					case OsMouseEvent::BUTTON:
						im->mouse()->set_button_state(ev.button, ev.pressed);
						break;
					case OsMouseEvent::MOVE:
						_mouse_curr_x = ev.x;
						_mouse_curr_y = ev.y;
						im->mouse()->set_axis(MouseAxis::CURSOR, vector3(ev.x, ev.y, 0.0f));
						break;
					case OsMouseEvent::WHEEL:
						im->mouse()->set_axis(MouseAxis::WHEEL, vector3(0.0f, ev.wheel, 0.0f));
						break;
					default:
						CE_FATAL("Unknown mouse event type");
						break;
				}
				break;
			}
			case OsEvent::KEYBOARD:
			{
				const OsKeyboardEvent& ev = event.keyboard;
				im->keyboard()->set_button_state(ev.button, ev.pressed);
				break;
			}
			case OsEvent::JOYPAD:
			{
				const OsJoypadEvent& ev = event.joypad;
				switch (ev.type)
				{
					case OsJoypadEvent::CONNECTED:
						im->joypad(ev.index)->set_connected(ev.connected);
						break;
					case OsJoypadEvent::BUTTON:
						im->joypad(ev.index)->set_button_state(ev.button, ev.pressed);
						break;
					case OsJoypadEvent::AXIS:
						im->joypad(ev.index)->set_axis(ev.button, vector3(ev.x, ev.y, ev.z));
						break;
					default:
						CE_FATAL("Unknown joypad event");
						break;
				}
				break;
			}
			case OsEvent::METRICS:
			{
				const OsMetricsEvent& ev = event.metrics;
				_width = ev.width;
				_height = ev.height;
				bgfx::reset(ev.width, ev.height, BGFX_RESET_VSYNC);
				break;
			}
			case OsEvent::EXIT:
			{
				exit = true;
				break;
			}
			case OsEvent::PAUSE:
			{
				pause();
				break;
			}
			case OsEvent::RESUME:
			{
				unpause();
				break;
			}
			default:
			{
				CE_FATAL("Unknown Os Event");
				break;
			}
		}
	}

	return exit;
}

void Device::run()
{
	_console_server = CE_NEW(_allocator, ConsoleServer)(default_allocator());

	namespace pcr = physics_config_resource;
	namespace phr = physics_resource;
	namespace pkr = package_resource;
	namespace sdr = sound_resource;
	namespace mhr = mesh_resource;
	namespace utr = unit_resource;
	namespace txr = texture_resource;
	namespace mtr = material_resource;
	namespace lur = lua_resource;
	namespace ftr = font_resource;
	namespace lvr = level_resource;
	namespace spr = sprite_resource;
	namespace shr = shader_resource;
	namespace sar = sprite_animation_resource;
	namespace cor = config_resource;

	bool do_continue = true;

#if CROWN_PLATFORM_LINUX || CROWN_PLATFORM_WINDOWS
	if (_device_options._do_compile || _device_options._server)
	{
		_bundle_compiler = CE_NEW(_allocator, BundleCompiler)();
		_bundle_compiler->register_compiler(RESOURCE_TYPE_SCRIPT,           RESOURCE_VERSION_SCRIPT,           lur::compile);
		_bundle_compiler->register_compiler(RESOURCE_TYPE_TEXTURE,          RESOURCE_VERSION_TEXTURE,          txr::compile);
		_bundle_compiler->register_compiler(RESOURCE_TYPE_MESH,             RESOURCE_VERSION_MESH,             mhr::compile);
		_bundle_compiler->register_compiler(RESOURCE_TYPE_SOUND,            RESOURCE_VERSION_SOUND,            sdr::compile);
		_bundle_compiler->register_compiler(RESOURCE_TYPE_UNIT,             RESOURCE_VERSION_UNIT,             utr::compile);
		_bundle_compiler->register_compiler(RESOURCE_TYPE_SPRITE,           RESOURCE_VERSION_SPRITE,           spr::compile);
		_bundle_compiler->register_compiler(RESOURCE_TYPE_PACKAGE,          RESOURCE_VERSION_PACKAGE,          pkr::compile);
		_bundle_compiler->register_compiler(RESOURCE_TYPE_PHYSICS,          RESOURCE_VERSION_PHYSICS,          phr::compile);
		_bundle_compiler->register_compiler(RESOURCE_TYPE_MATERIAL,         RESOURCE_VERSION_MATERIAL,         mtr::compile);
		_bundle_compiler->register_compiler(RESOURCE_TYPE_PHYSICS_CONFIG,   RESOURCE_VERSION_PHYSICS_CONFIG,   pcr::compile);
		_bundle_compiler->register_compiler(RESOURCE_TYPE_FONT,             RESOURCE_VERSION_FONT,             ftr::compile);
		_bundle_compiler->register_compiler(RESOURCE_TYPE_LEVEL,            RESOURCE_VERSION_LEVEL,            lvr::compile);
		_bundle_compiler->register_compiler(RESOURCE_TYPE_SHADER,           RESOURCE_VERSION_SHADER,           shr::compile);
		_bundle_compiler->register_compiler(RESOURCE_TYPE_SPRITE_ANIMATION, RESOURCE_VERSION_SPRITE_ANIMATION, sar::compile);
		_bundle_compiler->register_compiler(RESOURCE_TYPE_CONFIG,           RESOURCE_VERSION_CONFIG,           cor::compile);

		_bundle_compiler->scan(_device_options._source_dir);

		if (_device_options._server)
		{
			_console_server->register_command(StringId32("compile"), console_command_compile, _bundle_compiler);
			_console_server->listen(CROWN_DEFAULT_COMPILER_PORT, false);

			while (true)
			{
				_console_server->update();
				os::sleep(60);
			}
		}
		else
		{
			const char* source_dir = _device_options._source_dir;
			const char* bundle_dir = _device_options._bundle_dir;
			const char* platform = _device_options._platform;
			do_continue = _bundle_compiler->compile(bundle_dir, platform);
			do_continue = do_continue && _device_options._do_continue;
		}
	}
#endif // CROWN_PLATFORM_LINUX || CROWN_PLATFORM_WINDOWS

	if (do_continue)
	{
#if CROWN_PLATFORM_ANDROID
		_bundle_filesystem = CE_NEW(_allocator, ApkFilesystem)(default_allocator(), const_cast<AAssetManager*>((AAssetManager*)_device_options._asset_manager));
#else
		const char* bundle_dir = _device_options._bundle_dir;
		if (!bundle_dir)
		{
			char buf[1024];
			bundle_dir = os::getcwd(buf, sizeof(buf));
		}
		_bundle_filesystem = CE_NEW(_allocator, DiskFilesystem)(default_allocator());
		((DiskFilesystem*)_bundle_filesystem)->set_prefix(bundle_dir);
		if (!_bundle_filesystem->exists(bundle_dir))
			_bundle_filesystem->create_directory(bundle_dir);

		_last_log = _bundle_filesystem->open(CROWN_LAST_LOG, FileOpenMode::WRITE);
#endif // CROWN_PLATFORM_ANDROID

		_console_server->register_command(StringId32("script"), console_command_script, NULL);
		_console_server->register_command(StringId32("reload"), console_command_reload, NULL);
		_console_server->register_command(StringId32("pause"), console_command_pause, NULL);
		_console_server->register_command(StringId32("unpause"), console_command_unpause, NULL);
		_console_server->listen(_device_options._console_port, _device_options._wait_console);

		CE_LOGI("Initializing Crown Engine %s...", version());

		profiler_globals::init();

		_resource_loader  = CE_NEW(_allocator, ResourceLoader)(*_bundle_filesystem);
		_resource_manager = CE_NEW(_allocator, ResourceManager)(*_resource_loader);
		_resource_manager->register_type(RESOURCE_TYPE_SCRIPT,           lur::load, lur::unload, NULL,        NULL        );
		_resource_manager->register_type(RESOURCE_TYPE_TEXTURE,          txr::load, txr::unload, txr::online, txr::offline);
		_resource_manager->register_type(RESOURCE_TYPE_MESH,             mhr::load, mhr::unload, mhr::online, mhr::offline);
		_resource_manager->register_type(RESOURCE_TYPE_SOUND,            sdr::load, sdr::unload, NULL,        NULL        );
		_resource_manager->register_type(RESOURCE_TYPE_UNIT,             utr::load, utr::unload, NULL,        NULL        );
		_resource_manager->register_type(RESOURCE_TYPE_SPRITE,           spr::load, spr::unload, spr::online, spr::offline);
		_resource_manager->register_type(RESOURCE_TYPE_PACKAGE,          pkr::load, pkr::unload, NULL,        NULL        );
		_resource_manager->register_type(RESOURCE_TYPE_PHYSICS,          phr::load, phr::unload, NULL,        NULL        );
		_resource_manager->register_type(RESOURCE_TYPE_MATERIAL,         mtr::load, mtr::unload, mtr::online, mtr::offline);
		_resource_manager->register_type(RESOURCE_TYPE_PHYSICS_CONFIG,   pcr::load, pcr::unload, NULL,        NULL        );
		_resource_manager->register_type(RESOURCE_TYPE_FONT,             ftr::load, ftr::unload, NULL,        NULL        );
		_resource_manager->register_type(RESOURCE_TYPE_LEVEL,            lvr::load, lvr::unload, NULL,        NULL        );
		_resource_manager->register_type(RESOURCE_TYPE_SHADER,           shr::load, shr::unload, shr::online, shr::offline);
		_resource_manager->register_type(RESOURCE_TYPE_SPRITE_ANIMATION, sar::load, sar::unload, NULL,        NULL        );
		_resource_manager->register_type(RESOURCE_TYPE_CONFIG,           cor::load, cor::unload, NULL,        NULL        );

		read_config();

		_bgfx_allocator = CE_NEW(_allocator, BgfxAllocator)(default_allocator());
		_bgfx_callback  = CE_NEW(_allocator, BgfxCallback)();

		_display = display::create(_allocator);
		_window = window::create(_allocator);
		_window->open(_device_options._window_x
			, _device_options._window_y
			, _config_window_w
			, _config_window_h
			, _device_options._parent_window
			);
		_window->bgfx_setup();

		bgfx::init(bgfx::RendererType::Count
			, BGFX_PCI_ID_NONE
			, 0
			, _bgfx_callback
			, _bgfx_allocator
			);

		_shader_manager   = CE_NEW(_allocator, ShaderManager)(default_allocator());
		_material_manager = CE_NEW(_allocator, MaterialManager)(default_allocator(), *_resource_manager);
		_input_manager    = CE_NEW(_allocator, InputManager)(default_allocator());
		_unit_manager     = CE_NEW(_allocator, UnitManager)(default_allocator());
		_lua_environment  = CE_NEW(_allocator, LuaEnvironment)();

		audio_globals::init();
		physics_globals::init(_allocator);

		_boot_package = create_resource_package(_boot_package_name);
		_boot_package->load();
		_boot_package->flush();

		_lua_environment->load_libs();
		_lua_environment->execute((LuaResource*)_resource_manager->get(RESOURCE_TYPE_SCRIPT, _boot_script_name));
		_lua_environment->call_global("init", 0);

		_last_time = os::clocktime();

		CE_LOGD("Engine initialized");

		while (!process_events() && !_quit)
		{
			_current_time = os::clocktime();
			const s64 time = _current_time - _last_time;
			_last_time = _current_time;
			const f64 freq = (f64) os::clockfrequency();
			_last_delta_time = f32(time * (1.0 / freq));
			_time_since_start += _last_delta_time;

			profiler_globals::clear();
			_console_server->update();

			RECORD_FLOAT("device.dt", _last_delta_time);
			RECORD_FLOAT("device.fps", 1.0f/_last_delta_time);

			if (!_paused)
			{
				_resource_manager->complete_requests();

				{
					const s64 t0 = os::clocktime();
					_lua_environment->call_global("update", 1, ARGUMENT_FLOAT, last_delta_time());
					const s64 t1 = os::clocktime();
					RECORD_FLOAT("lua.update", (t1 - t0)*(1.0 / freq));
				}
				{
					const s64 t0 = os::clocktime();
					_lua_environment->call_global("render", 1, ARGUMENT_FLOAT, last_delta_time());
					const s64 t1 = os::clocktime();
					RECORD_FLOAT("lua.render", (t1 - t0)*(1.0 / freq));
				}
			}

			_input_manager->update();

			const bgfx::Stats* stats = bgfx::getStats();
			RECORD_FLOAT("bgfx.gpu_time", f32(f64(stats->gpuTimeEnd - stats->gpuTimeBegin)*1000.0/stats->gpuTimerFreq));
			RECORD_FLOAT("bgfx.cpu_time", f32(f64(stats->cpuTimeEnd - stats->cpuTimeBegin)*1000.0/stats->cpuTimerFreq));

			bgfx::frame();
			profiler_globals::flush();

			_lua_environment->reset_temporaries();

			_frame_count++;
		}

		_lua_environment->call_global("shutdown", 0);

		_boot_package->unload();
		destroy_resource_package(*_boot_package);

		physics_globals::shutdown(_allocator);
		audio_globals::shutdown();

		CE_DELETE(_allocator, _lua_environment);
		CE_DELETE(_allocator, _unit_manager);
		CE_DELETE(_allocator, _input_manager);
		CE_DELETE(_allocator, _material_manager);
		CE_DELETE(_allocator, _shader_manager);
		CE_DELETE(_allocator, _resource_manager);
		CE_DELETE(_allocator, _resource_loader);

		bgfx::shutdown();
		window::destroy(_allocator, *_window);
		display::destroy(_allocator, *_display);
		CE_DELETE(_allocator, _bgfx_callback);
		CE_DELETE(_allocator, _bgfx_allocator);

		if (_last_log)
			_bundle_filesystem->close(*_last_log);

		CE_DELETE(_allocator, _bundle_filesystem);

		profiler_globals::shutdown();
	}

	_console_server->shutdown();
	CE_DELETE(_allocator, _console_server);
	CE_DELETE(_allocator, _bundle_compiler);

	_allocator.clear();
}

void Device::quit()
{
	_quit = true;
}

void Device::pause()
{
	_paused = true;
	CE_LOGI("Engine paused.");
}

void Device::unpause()
{
	_paused = false;
	CE_LOGI("Engine unpaused.");
}

void Device::resolution(u16& width, u16& height)
{
	width = _width;
	height = _height;
}

u64 Device::frame_count() const
{
	return _frame_count;
}

f32 Device::last_delta_time() const
{
	return _last_delta_time;
}

f64 Device::time_since_start() const
{
	return _time_since_start;
}

void Device::render(World& world, CameraInstance camera)
{
	bgfx::setViewClear(0
		, BGFX_CLEAR_COLOR | BGFX_CLEAR_DEPTH
		, 0x353839FF
		, 1.0f
		, 0
		);

	bgfx::setViewRect(0, 0, 0, _width, _height);
	bgfx::setViewRect(1, 0, 0, _width, _height);
	bgfx::setViewRect(2, 0, 0, _width, _height);

	const f32* view = to_float_ptr(world.camera_view_matrix(camera));
	const f32* proj = to_float_ptr(world.camera_projection_matrix(camera));

	bgfx::setViewTransform(0, view, proj);
	bgfx::setViewTransform(1, view, proj);
	bgfx::setViewTransform(2, to_float_ptr(MATRIX4X4_IDENTITY), to_float_ptr(MATRIX4X4_IDENTITY));
	bgfx::setViewSeq(2, true);

	bgfx::touch(0);
	bgfx::touch(1);
	bgfx::touch(2);

	world.set_camera_viewport_metrics(camera, 0, 0, _width, _height);

	world.render(camera);
}

World* Device::create_world()
{
	World* w = CE_NEW(default_allocator(), World)(default_allocator()
		, *_resource_manager
		, *_shader_manager
		, *_material_manager
		, *_unit_manager
		, *_lua_environment
		);
	array::push_back(_worlds, w);
	return w;
}

void Device::destroy_world(World& w)
{
	for (u32 i = 0, n = array::size(_worlds); i < n; ++i)
	{
		if (&w == _worlds[i])
		{
			CE_DELETE(default_allocator(), &w);
			_worlds[i] = _worlds[n - 1];
			array::pop_back(_worlds);
			return;
		}
	}

	CE_ASSERT(false, "Bad world");
}

ResourcePackage* Device::create_resource_package(StringId64 id)
{
	return CE_NEW(default_allocator(), ResourcePackage)(id, *_resource_manager);
}

void Device::destroy_resource_package(ResourcePackage& rp)
{
	CE_DELETE(default_allocator(), &rp);
}

void Device::reload(StringId64 type, StringId64 name)
{
	const void* old_resource = _resource_manager->get(type, name);
	_resource_manager->reload(type, name);
	const void* new_resource = _resource_manager->get(type, name);

	if (type == RESOURCE_TYPE_SCRIPT)
	{
		_lua_environment->execute((const LuaResource*)new_resource);
	}
}

static StringStream& sanitize(StringStream& ss, const char* msg)
{
	using namespace string_stream;
	const char* ch = msg;
	for (; *ch; ch++)
	{
		if (*ch == '"' || *ch == '\\')
			ss << "\\";
		ss << *ch;
	}

	return ss;
}

static const char* s_severity_map[] = { "info", "warning", "error", "debug" };
CE_STATIC_ASSERT(CE_COUNTOF(s_severity_map) == LogSeverity::COUNT);

void Device::log(const char* msg, LogSeverity::Enum severity)
{
	if (_last_log)
	{
		_last_log->write(msg, strlen32(msg));
		_last_log->write("\n", 1);
		_last_log->flush();
	}

	if (_console_server)
	{
		TempAllocator4096 ta;
		StringStream json(ta);

		json << "{\"type\":\"message\",";
		json << "\"severity\":\"" << s_severity_map[severity] << "\",";
		json << "\"message\":\""; sanitize(json, msg) << "\"}";

		_console_server->send(string_stream::c_str(json));
	}
}

ConsoleServer* Device::console_server()
{
	return _console_server;
}

BundleCompiler* Device::bundle_compiler()
{
	return _bundle_compiler;
}

ResourceManager* Device::resource_manager()
{
	return _resource_manager;
}

LuaEnvironment* Device::lua_environment()
{
	return _lua_environment;
}

InputManager* Device::input_manager()
{
	return _input_manager;
}

ShaderManager* Device::shader_manager()
{
	return _shader_manager;
}

MaterialManager* Device::material_manager()
{
	return _material_manager;
}

UnitManager* Device::unit_manager()
{
	return _unit_manager;
}

Display* Device::display()
{
	return _display;
}

Window* Device::window()
{
	return _window;
}

char _buffer[sizeof(Device)];
Device* _device = NULL;

void run(const DeviceOptions& opts)
{
	CE_ASSERT(_device == NULL, "Crown already initialized");
	_device = new (_buffer) Device(opts);
	_device->run();
	_device->~Device();
	_device = NULL;
}

Device* device()
{
	return crown::_device;
}

} // namespace crown
