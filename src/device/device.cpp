/*
 * Copyright (c) 2012-2016 Daniele Bartolini and individual contributors.
 * License: https://github.com/taylor001/crown/blob/master/LICENSE
 */

#include "apk_filesystem.h"
#include "array.h"
#include "audio.h"
#include "bundle_compiler.h"
#include "config.h"
#include "console_server.h"
#include "device.h"
#include "disk_filesystem.h"
#include "file.h"
#include "filesystem.h"
#include "input_device.h"
#include "input_manager.h"
#include "log.h"
#include "lua_environment.h"
#include "map.h"
#include "material_manager.h"
#include "matrix4x4.h"
#include "memory.h"
#include "os.h"
#include "os_event_queue.h"
#include "path.h"
#include "physics.h"
#include "profiler.h"
#include "proxy_allocator.h"
#include "resource_loader.h"
#include "resource_manager.h"
#include "resource_package.h"
#include "shader_manager.h"
#include "sjson.h"
#include "string_stream.h"
#include "string_utils.h"
#include "temp_allocator.h"
#include "types.h"
#include "unit_manager.h"
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

void Device::run()
{
	profiler_globals::init();

	_console_server = CE_NEW(_allocator, ConsoleServer)(default_allocator());
	_console_server->listen(_device_options._console_port, _device_options._wait_console);

	bool do_continue = true;

#if CROWN_PLATFORM_LINUX || CROWN_PLATFORM_WINDOWS
	if (_device_options._do_compile)
	{
		_bundle_compiler = CE_NEW(_allocator, BundleCompiler)(_device_options._source_dir, _device_options._bundle_dir);
		bool success = _bundle_compiler->compile_all(_device_options._platform);
		do_continue = success && _device_options._do_continue;
	}
#endif // CROWN_PLATFORM_LINUX || CROWN_PLATFORM_WINDOWS

	if (do_continue)
	{
#if CROWN_PLATFORM_ANDROID
		_bundle_filesystem = CE_NEW(_allocator, ApkFilesystem)(default_allocator(), const_cast<AAssetManager*>((AAssetManager*)_device_options._asset_manager));
#else
		_bundle_filesystem = CE_NEW(_allocator, DiskFilesystem)(default_allocator(), _device_options._bundle_dir);
		_last_log = _bundle_filesystem->open(CROWN_LAST_LOG, FileOpenMode::WRITE);
#endif // CROWN_PLATFORM_ANDROID

		CE_LOGI("Initializing Crown Engine %s...", version());

		_resource_loader  = CE_NEW(_allocator, ResourceLoader)(*_bundle_filesystem);
		_resource_manager = CE_NEW(_allocator, ResourceManager)(*_resource_loader);

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
				_lua_environment->call_global("update", 1, ARGUMENT_FLOAT, last_delta_time());
				_lua_environment->call_global("render", 1, ARGUMENT_FLOAT, last_delta_time());
			}

			_input_manager->update();

			const bgfx::Stats* stats = bgfx::getStats();
			RECORD_FLOAT("bgfx.gpu_time", f64(stats->gpuTimeEnd - stats->gpuTimeBegin)*1000.0/stats->gpuTimerFreq);
			RECORD_FLOAT("bgfx.cpu_time", f64(stats->cpuTimeEnd - stats->cpuTimeBegin)*1000.0/stats->cpuTimerFreq);

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
		{
			_bundle_filesystem->close(*_last_log);
		}

		CE_DELETE(_allocator, _bundle_filesystem);
	}

	CE_DELETE(_allocator, _bundle_compiler);

	_console_server->shutdown();
	CE_DELETE(_allocator, _console_server);

	profiler_globals::shutdown();

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
		if (*ch == '"')
			ss << "\\";
		ss << *ch;
	}

	return ss;
}

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
		static const char* stt[] = { "info", "warning", "error", "debug" };

		using namespace string_stream;
		TempAllocator4096 ta;
		StringStream json(ta);

		json << "{\"type\":\"message\",";
		json << "\"severity\":\"" << stt[severity] << "\",";
		json << "\"message\":\""; sanitize(json, msg) << "\"}";

		_console_server->send(c_str(json));
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
