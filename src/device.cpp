/*
 * Copyright (c) 2012-2016 Daniele Bartolini and individual contributors.
 * License: https://github.com/taylor001/crown/blob/master/LICENSE
 */

#include "array.h"
#include "audio.h"
#include "config.h"
#include "console_server.h"
#include "device.h"
#include "disk_filesystem.h"
#include "filesystem.h"
#include "input_device.h"
#include "input_manager.h"
#include "log.h"
#include "lua_environment.h"
#include "map.h"
#include "material_manager.h"
#include "memory.h"
#include "os.h"
#include "path.h"
#include "physics.h"
#include "profiler.h"
#include "proxy_allocator.h"
#include "resource_loader.h"
#include "resource_manager.h"
#include "resource_package.h"
#include "shader_manager.h"
#include "sjson.h"
#include "string_utils.h"
#include "types.h"
#include "unit_manager.h"
#include "vector3.h"
#include "world.h"
#include <bgfx/bgfx.h>
#include <bx/allocator.h>

#if CROWN_PLATFORM_ANDROID
	#include "apk_filesystem.h"
#endif // CROWN_PLATFORM_ANDROID

#define MAX_SUBSYSTEMS_HEAP 8 * 1024 * 1024

namespace crown
{

struct BgfxCallback : public bgfx::CallbackI
{
	virtual void fatal(bgfx::Fatal::Enum _code, const char* _str)
	{
		CE_ASSERT(false, "Fatal error: 0x%08x: %s", _code, _str);
	}

	virtual void traceVargs(const char* /*_filePath*/, uint16_t /*_line*/, const char* _format, va_list _argList)
	{
		char buf[2048];
		strncpy(buf, _format, sizeof(buf));
		buf[strlen32(buf)-1] = '\0'; // Remove trailing newline
		CE_LOGDV(buf, _argList);
	}

	virtual uint32_t cacheReadSize(uint64_t /*_id*/)
	{
		return 0;
	}

	virtual bool cacheRead(uint64_t /*_id*/, void* /*_data*/, uint32_t /*_size*/)
	{
		return false;
	}

	virtual void cacheWrite(uint64_t /*_id*/, const void* /*_data*/, uint32_t /*_size*/)
	{
	}

	virtual void screenShot(const char* /*_filePath*/, uint32_t /*_width*/, uint32_t /*_height*/, uint32_t /*_pitch*/, const void* /*_data*/, uint32_t /*_size*/, bool /*_yflip*/)
	{
	}

	virtual void captureBegin(uint32_t /*_width*/, uint32_t /*_height*/, uint32_t /*_pitch*/, bgfx::TextureFormat::Enum /*_format*/, bool /*_yflip*/)
	{
	}

	virtual void captureEnd()
	{
	}

	virtual void captureFrame(const void* /*_data*/, uint32_t /*_size*/)
	{
	}
};

struct BgfxAllocator : public bx::AllocatorI
{
	BgfxAllocator(Allocator& a)
		: _allocator("bgfx", a)
	{
	}

	virtual void* realloc(void* _ptr, size_t _size, size_t _align, const char* /*_file*/, uint32_t /*_line*/)
	{
		if (!_ptr)
			return _allocator.allocate((uint32_t)_size, (uint32_t)_align == 0 ? 1 : (uint32_t)_align);

		if (_size == 0)
		{
			_allocator.deallocate(_ptr);
			return NULL;
		}

		// Realloc
		void* p = _allocator.allocate((uint32_t)_size, (uint32_t)_align == 0 ? 1 : (uint32_t)_align);
		_allocator.deallocate(_ptr);
		return p;
	}

private:

	ProxyAllocator _allocator;
};

Device::Device(const DeviceOptions& opts)
	: _allocator(default_allocator(), MAX_SUBSYSTEMS_HEAP)
	, _device_options(opts)
	, _bundle_filesystem(NULL)
	, _resource_loader(NULL)
	, _resource_manager(NULL)
	, _bgfx_allocator(NULL)
	, _bgfx_callback(NULL)
	, _shader_manager(NULL)
	, _material_manager(NULL)
	, _input_manager(NULL)
	, _unit_manager(NULL)
	, _lua_environment(NULL)
	, _window(NULL)
	, _boot_package_id(uint64_t(0))
	, _boot_script_id(uint64_t(0))
	, _boot_package(NULL)
	, _worlds(default_allocator())
	, _width(0)
	, _height(0)
	, _mouse_curr_x(0)
	, _mouse_curr_y(0)
	, _mouse_last_x(0)
	, _mouse_last_y(0)
	, _is_init(false)
	, _is_running(false)
	, _is_paused(false)
	, _frame_count(0)
	, _last_time(0)
	, _current_time(0)
	, _last_delta_time(0.0f)
	, _time_since_start(0.0)
{
}

void Device::init()
{
	// Initialize
	CE_LOGI("Initializing Crown Engine %s...", version());

	profiler_globals::init();

#if CROWN_PLATFORM_ANDROID
	_bundle_filesystem = CE_NEW(_allocator, ApkFilesystem)(const_cast<AAssetManager*>(_device_options.asset_manager()));
#else
	_bundle_filesystem = CE_NEW(_allocator, DiskFilesystem)(_device_options.bundle_dir());
#endif // CROWN_PLATFORM_ANDROID

	_resource_loader  = CE_NEW(_allocator, ResourceLoader)(*_bundle_filesystem);
	_resource_manager = CE_NEW(_allocator, ResourceManager)(*_resource_loader);

	read_config();

	_bgfx_allocator = CE_NEW(_allocator, BgfxAllocator)(default_allocator());
	_bgfx_callback  = CE_NEW(_allocator, BgfxCallback)();

	_window = Window::create(_allocator);
	_window->open(_device_options.window_x()
		, _device_options.window_y()
		, _device_options.window_width()
		, _device_options.window_height()
		, _device_options.parent_window()
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
	physics_globals::init();

	_boot_package = create_resource_package(_boot_package_id);
	_boot_package->load();
	_boot_package->flush();

	_lua_environment->load_libs();
	_lua_environment->execute((LuaResource*)_resource_manager->get(SCRIPT_TYPE, _boot_script_id));
	_lua_environment->call_global("init", 0);

	_is_init = true;
	_is_running = true;
	_last_time = os::clocktime();

	CE_LOGD("Engine initialized");
}

void Device::shutdown()
{
	CE_ASSERT(_is_init, "Engine is not initialized");

	_is_running = false;
	_is_init = false;

	_lua_environment->call_global("shutdown", 0);

	_boot_package->unload();
	destroy_resource_package(*_boot_package);

	physics_globals::shutdown();
	audio_globals::shutdown();

	CE_DELETE(_allocator, _lua_environment);
	CE_DELETE(_allocator, _unit_manager);
	CE_DELETE(_allocator, _input_manager);
	CE_DELETE(_allocator, _material_manager);
	CE_DELETE(_allocator, _shader_manager);
	CE_DELETE(_allocator, _resource_manager);
	CE_DELETE(_allocator, _resource_loader);
	CE_DELETE(_allocator, _bundle_filesystem);

	bgfx::shutdown();
	Window::destroy(_allocator, *_window);
	CE_DELETE(_allocator, _bgfx_callback);
	CE_DELETE(_allocator, _bgfx_allocator);

	profiler_globals::shutdown();

	_allocator.clear();
}

void Device::quit()
{
	_is_running = false;
}

void Device::pause()
{
	_is_paused = true;
	CE_LOGI("Engine paused.");
}

void Device::unpause()
{
	_is_paused = false;
	CE_LOGI("Engine unpaused.");
}

void Device::resolution(uint16_t& width, uint16_t& height)
{
	width = _width;
	height = _height;
}

bool Device::is_running() const
{
	return _is_running;
}

uint64_t Device::frame_count() const
{
	return _frame_count;
}

float Device::last_delta_time() const
{
	return _last_delta_time;
}

double Device::time_since_start() const
{
	return _time_since_start;
}

void Device::update()
{
	while (!process_events() && _is_running)
	{
		_current_time = os::clocktime();
		const int64_t time = _current_time - _last_time;
		_last_time = _current_time;
		const double freq = (double) os::clockfrequency();
		_last_delta_time = time * (1.0 / freq);
		_time_since_start += _last_delta_time;

		profiler_globals::clear();
		console_server_globals::update();

		RECORD_FLOAT("device.dt", _last_delta_time);
		RECORD_FLOAT("device.fps", 1.0f/_last_delta_time);

		if (!_is_paused)
		{
			_resource_manager->complete_requests();
			_lua_environment->call_global("update", 1, ARGUMENT_FLOAT, last_delta_time());
			_lua_environment->call_global("render", 1, ARGUMENT_FLOAT, last_delta_time());
		}

		_input_manager->update();

		const bgfx::Stats* stats = bgfx::getStats();
		RECORD_FLOAT("bgfx.gpu_time", double(stats->gpuTimeEnd - stats->gpuTimeBegin)*1000.0/stats->gpuTimerFreq);
		RECORD_FLOAT("bgfx.cpu_time", double(stats->cpuTimeEnd - stats->cpuTimeBegin)*1000.0/stats->cpuTimerFreq);

		bgfx::frame();
		profiler_globals::flush();

		_lua_environment->reset_temporaries();

		_frame_count++;
	}
}

void Device::render_world(World& world, CameraInstance camera)
{
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
	for (uint32_t i = 0, n = array::size(_worlds); i < n; ++i)
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

	if (type == SCRIPT_TYPE)
	{
		_lua_environment->execute((const LuaResource*)new_resource);
	}
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

Window* Device::window()
{
	return _window;
}

void Device::read_config()
{
	TempAllocator4096 ta;
	DynamicString boot_dir(ta);

	if (_device_options.boot_dir() != NULL)
	{
		boot_dir += _device_options.boot_dir();
		boot_dir += '/';
	}

	boot_dir += CROWN_BOOT_CONFIG;

	const StringId64 config_name(boot_dir.c_str());

	_resource_manager->load(CONFIG_TYPE, config_name);
	_resource_manager->flush();
	const char* cfile = (const char*)_resource_manager->get(CONFIG_TYPE, config_name);

	JsonObject config(ta);
	sjson::parse(cfile, config);

	_boot_script_id  = sjson::parse_resource_id(config["boot_script"]);
	_boot_package_id = sjson::parse_resource_id(config["boot_package"]);

	_resource_manager->unload(CONFIG_TYPE, config_name);
}

bool Device::process_events()
{
	OsEvent event;
	bool exit = false;
	InputManager* im = _input_manager;

	const int16_t dt_x = _mouse_curr_x - _mouse_last_x;
	const int16_t dt_y = _mouse_curr_y - _mouse_last_y;
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

void init(const DeviceOptions& opts)
{
	CE_ASSERT(_device == NULL, "Crown already initialized");
	_device = new (_buffer) Device(opts);
	_device->init();
}

void update()
{
	_device->update();
}

void shutdown()
{
	_device->shutdown();
	_device->~Device();
	_device = NULL;
}

Device* device()
{
	return crown::_device;
}

} // namespace crown
