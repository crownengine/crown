/*
 * Copyright (c) 2012-2017 Daniele Bartolini and individual contributors.
 * License: https://github.com/taylor001/crown/blob/master/LICENSE
 */

#include "array.h"
#include "audio.h"
#include "config.h"
#include "config_resource.h"
#include "console_server.h"
#include "device.h"
#include "device_event_queue.h"
#include "file.h"
#include "filesystem.h"
#include "filesystem_apk.h"
#include "filesystem_disk.h"
#include "font_resource.h"
#include "input_device.h"
#include "input_manager.h"
#include "json_object.h"
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
#include "state_machine_resource.h"
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

namespace { const crown::log_internal::System DEVICE = { "Device" }; }

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
		logiv(DEVICE, buf, _argList);
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
	ProxyAllocator _allocator;

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
};

static void console_command_script(ConsoleServer& /*cs*/, TCPSocket /*client*/, const char* json, void* user_data)
{
	TempAllocator4096 ta;
	JsonObject obj(ta);
	DynamicString script(ta);

	sjson::parse(json, obj);
	sjson::parse_string(obj["script"], script);

	((Device*)user_data)->_lua_environment->execute_string(script.c_str());
}

static void console_command(ConsoleServer& cs, TCPSocket client, const char* json, void* user_data)
{
	TempAllocator4096 ta;
	JsonObject obj(ta);
	JsonArray args(ta);

	sjson::parse(json, obj);
	sjson::parse_array(obj["args"], args);

	DynamicString cmd(ta);
	sjson::parse_string(args[0], cmd);

	if (cmd == "pause")
		device()->pause();
	else if (cmd == "unpause")
		device()->unpause();
	else if (cmd == "reload")
	{
		if (array::size(args) != 3)
		{
			cs.error(client, "Usage: reload type name");
			return;
		}

		DynamicString type(ta);
		DynamicString name(ta);
		sjson::parse_string(args[1], type);
		sjson::parse_string(args[2], name);

		((Device*)user_data)->reload(ResourceId(type.c_str()), ResourceId(name.c_str()));
	}
}

Device::Device(const DeviceOptions& opts, ConsoleServer& cs)
	: _allocator(default_allocator(), MAX_SUBSYSTEMS_HEAP)
	, _device_options(opts)
	, _boot_config(default_allocator())
	, _console_server(&cs)
	, _data_filesystem(NULL)
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
	, _worlds(default_allocator())
	, _width(0)
	, _height(0)
	, _quit(false)
	, _paused(false)
	, _frame_count(0)
	, _time_since_start(0.0)
{
}

bool Device::process_events(bool vsync)
{
	InputManager* im = _input_manager;
	bool exit = false;
	bool reset = false;

	OsEvent event;
	while(next_event(event))
	{
		if (event.type == OsEventType::NONE)
			continue;

		switch (event.type)
		{
		case OsEventType::BUTTON:
			{
				const ButtonEvent ev = event.button;
				switch (ev.device_id)
				{
				case InputDeviceType::KEYBOARD:
					im->keyboard()->set_button_state(ev.button_num, ev.pressed);
					break;

				case InputDeviceType::MOUSE:
					im->mouse()->set_button_state(ev.button_num, ev.pressed);
					break;

				case InputDeviceType::TOUCHSCREEN:
					im->touch()->set_button_state(ev.button_num, ev.pressed);
					break;

				case InputDeviceType::JOYPAD:
					im->joypad(ev.device_num)->set_button_state(ev.button_num, ev.pressed);
					break;
				}
			}
			break;

		case OsEventType::AXIS:
			{
				const AxisEvent ev = event.axis;
				switch (ev.device_id)
				{
				case InputDeviceType::MOUSE:
					im->mouse()->set_axis(ev.axis_num, vector3(ev.axis_x, ev.axis_y, ev.axis_z));
					break;

				case InputDeviceType::JOYPAD:
					im->joypad(ev.device_num)->set_axis(ev.axis_num, vector3(ev.axis_x, ev.axis_y, ev.axis_z));
					break;
				}
			}
			break;

		case OsEventType::STATUS:
			{
				const StatusEvent ev = event.status;
				switch (ev.device_id)
				{
				case InputDeviceType::JOYPAD:
					im->joypad(ev.device_num)->set_connected(ev.connected);
					break;
				}
			}
			break;

		case OsEventType::RESOLUTION:
			{
				const ResolutionEvent& ev = event.resolution;
				_width  = ev.width;
				_height = ev.height;
				reset   = true;
			}
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

		default:
			CE_FATAL("Unknown OS event");
			break;
		}
	}

	if (reset)
		bgfx::reset(_width, _height, (vsync ? BGFX_RESET_VSYNC : BGFX_RESET_NONE));

	return exit;
}

void Device::run()
{
	_console_server->register_command("command", console_command, this);
	_console_server->register_command("script",  console_command_script, this);

	_console_server->listen(_device_options._console_port, _device_options._wait_console);

#if CROWN_PLATFORM_ANDROID
	_data_filesystem = CE_NEW(_allocator, FilesystemApk)(default_allocator(), const_cast<AAssetManager*>((AAssetManager*)_device_options._asset_manager));
#else
	const char* data_dir = _device_options._data_dir.c_str();
	if (!data_dir)
	{
		char buf[1024];
		data_dir = os::getcwd(buf, sizeof(buf));
	}
	_data_filesystem = CE_NEW(_allocator, FilesystemDisk)(default_allocator());
	((FilesystemDisk*)_data_filesystem)->set_prefix(data_dir);
	if (!_data_filesystem->exists(data_dir))
		_data_filesystem->create_directory(data_dir);

	_last_log = _data_filesystem->open(CROWN_LAST_LOG, FileOpenMode::WRITE);
#endif // CROWN_PLATFORM_ANDROID

	logi(DEVICE, "Initializing Crown Engine %s %s %s", CROWN_VERSION, CROWN_PLATFORM_NAME, CROWN_ARCH_NAME);

	profiler_globals::init();

	namespace smr = state_machine_internal;
	namespace cor = config_resource_internal;
	namespace ftr = font_resource_internal;
	namespace lur = lua_resource_internal;
	namespace lvr = level_resource_internal;
	namespace mhr = mesh_resource_internal;
	namespace mtr = material_resource_internal;
	namespace pcr = physics_config_resource_internal;
	namespace phr = physics_resource_internal;
	namespace pkr = package_resource_internal;
	namespace sar = sprite_animation_resource_internal;
	namespace sdr = sound_resource_internal;
	namespace shr = shader_resource_internal;
	namespace spr = sprite_resource_internal;
	namespace txr = texture_resource_internal;
	namespace utr = unit_resource_internal;

	_resource_loader  = CE_NEW(_allocator, ResourceLoader)(*_data_filesystem);
	_resource_manager = CE_NEW(_allocator, ResourceManager)(*_resource_loader);
	_resource_manager->register_type(RESOURCE_TYPE_CONFIG,           RESOURCE_VERSION_CONFIG,           cor::load, cor::unload, NULL,        NULL        );
	_resource_manager->register_type(RESOURCE_TYPE_FONT,             RESOURCE_VERSION_FONT,             NULL,      NULL,        NULL,        NULL        );
	_resource_manager->register_type(RESOURCE_TYPE_LEVEL,            RESOURCE_VERSION_LEVEL,            NULL,      NULL,        NULL,        NULL        );
	_resource_manager->register_type(RESOURCE_TYPE_MATERIAL,         RESOURCE_VERSION_MATERIAL,         mtr::load, mtr::unload, mtr::online, mtr::offline);
	_resource_manager->register_type(RESOURCE_TYPE_MESH,             RESOURCE_VERSION_MESH,             mhr::load, mhr::unload, mhr::online, mhr::offline);
	_resource_manager->register_type(RESOURCE_TYPE_PACKAGE,          RESOURCE_VERSION_PACKAGE,          pkr::load, pkr::unload, NULL,        NULL        );
	_resource_manager->register_type(RESOURCE_TYPE_PHYSICS,          RESOURCE_VERSION_PHYSICS,          NULL,      NULL,        NULL,        NULL        );
	_resource_manager->register_type(RESOURCE_TYPE_PHYSICS_CONFIG,   RESOURCE_VERSION_PHYSICS_CONFIG,   NULL,      NULL,        NULL,        NULL        );
	_resource_manager->register_type(RESOURCE_TYPE_SCRIPT,           RESOURCE_VERSION_SCRIPT,           NULL,      NULL,        NULL,        NULL        );
	_resource_manager->register_type(RESOURCE_TYPE_SHADER,           RESOURCE_VERSION_SHADER,           shr::load, shr::unload, shr::online, shr::offline);
	_resource_manager->register_type(RESOURCE_TYPE_SOUND,            RESOURCE_VERSION_SOUND,            NULL,      NULL,        NULL,        NULL        );
	_resource_manager->register_type(RESOURCE_TYPE_SPRITE,           RESOURCE_VERSION_SPRITE,           NULL,      NULL,        NULL,        NULL        );
	_resource_manager->register_type(RESOURCE_TYPE_SPRITE_ANIMATION, RESOURCE_VERSION_SPRITE_ANIMATION, NULL,      NULL,        NULL,        NULL        );
	_resource_manager->register_type(RESOURCE_TYPE_STATE_MACHINE,    RESOURCE_VERSION_STATE_MACHINE,    NULL,      NULL,        NULL,        NULL        );
	_resource_manager->register_type(RESOURCE_TYPE_TEXTURE,          RESOURCE_VERSION_TEXTURE,          txr::load, txr::unload, txr::online, txr::offline);
	_resource_manager->register_type(RESOURCE_TYPE_UNIT,             RESOURCE_VERSION_UNIT,             NULL,      NULL,        NULL,        NULL        );

	// Read config
	{
		TempAllocator512 ta;
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
		_boot_config.parse((const char*)_resource_manager->get(RESOURCE_TYPE_CONFIG, config_name));
		_resource_manager->unload(RESOURCE_TYPE_CONFIG, config_name);
	}

	// Init all remaining subsystems
	_bgfx_allocator = CE_NEW(_allocator, BgfxAllocator)(default_allocator());
	_bgfx_callback  = CE_NEW(_allocator, BgfxCallback)();

	_display = display::create(_allocator);
	_window = window::create(_allocator);
	_window->open(_device_options._window_x
		, _device_options._window_y
		, _boot_config.window_w
		, _boot_config.window_h
		, _device_options._parent_window
		);
	_window->set_title(_boot_config.window_title.c_str());
	_window->set_fullscreen(_boot_config.fullscreen);
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

	ResourcePackage* boot_package = create_resource_package(_boot_config.boot_package_name);
	boot_package->load();
	boot_package->flush();

	_lua_environment->load_libs();
	_lua_environment->execute_string(_device_options._lua_string.c_str());
	_lua_environment->execute((LuaResource*)_resource_manager->get(RESOURCE_TYPE_SCRIPT, _boot_config.boot_script_name));

	logi(DEVICE, "Initialized");

	_lua_environment->call_global("init", 0);

	s64 last_time = os::clocktime();
	s64 curr_time;
	f32 dt = 0.0f;

	while (!process_events(_boot_config.vsync) && !_quit)
	{
		curr_time = os::clocktime();
		const s64 time = curr_time - last_time;
		last_time = curr_time;
		const f64 freq = (f64)os::clockfrequency();
		dt = f32(time * (1.0 / freq));
		_time_since_start += dt;

		profiler_globals::clear();
		_console_server->update();

		RECORD_FLOAT("device.dt", dt);
		RECORD_FLOAT("device.fps", 1.0f/dt);

		if (!_paused)
		{
			_resource_manager->complete_requests();

			{
				const s64 t0 = os::clocktime();
				_lua_environment->call_global("update", 1, ARGUMENT_FLOAT, dt);
				const s64 t1 = os::clocktime();
				RECORD_FLOAT("lua.update", f32((t1 - t0)*(1.0 / freq)));
			}
			{
				const s64 t0 = os::clocktime();
				_lua_environment->call_global("render", 1, ARGUMENT_FLOAT, dt);
				const s64 t1 = os::clocktime();
				RECORD_FLOAT("lua.render", f32((t1 - t0)*(1.0 / freq)));
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

	boot_package->unload();
	destroy_resource_package(*boot_package);

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
	_window->close();
	window::destroy(_allocator, *_window);
	display::destroy(_allocator, *_display);
	CE_DELETE(_allocator, _bgfx_callback);
	CE_DELETE(_allocator, _bgfx_allocator);

	if (_last_log)
		_data_filesystem->close(*_last_log);

	CE_DELETE(_allocator, _data_filesystem);

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
	logi(DEVICE, "Paused");
}

void Device::unpause()
{
	_paused = false;
	logi(DEVICE, "Unpaused");
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

f64 Device::time_since_start() const
{
	return _time_since_start;
}

void Device::render(World& world, UnitId camera_unit)
{
	bgfx::setViewClear(0
		, BGFX_CLEAR_COLOR | BGFX_CLEAR_DEPTH
		, 0x353839ff
		, 1.0f
		, 0
		);

	bgfx::setViewRect(0, 0, 0, _width, _height);
	bgfx::setViewRect(1, 0, 0, _width, _height);
	bgfx::setViewRect(2, 0, 0, _width, _height);

	float aspect_ratio = (_boot_config.aspect_ratio == -1.0f
		? (float)_width/(float)_height
		: _boot_config.aspect_ratio
		);
	world.camera_set_aspect(camera_unit, aspect_ratio);
	world.camera_set_viewport_metrics(camera_unit, 0, 0, _width, _height);

	const Matrix4x4 view = world.camera_view_matrix(camera_unit);
	const Matrix4x4 proj = world.camera_projection_matrix(camera_unit);

	bgfx::setViewTransform(0, to_float_ptr(view), to_float_ptr(proj));
	bgfx::setViewTransform(1, to_float_ptr(view), to_float_ptr(proj));
	bgfx::setViewTransform(2, to_float_ptr(MATRIX4X4_IDENTITY), to_float_ptr(MATRIX4X4_IDENTITY));
	bgfx::setViewSeq(2, true);

	bgfx::touch(0);
	bgfx::touch(1);
	bgfx::touch(2);

	world.render(view, proj);
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

	CE_FATAL("Bad world");
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
	TempAllocator64 ta;
	DynamicString type_str(ta);
	DynamicString name_str(ta);
	type.to_string(type_str);
	name.to_string(name_str);

	logi(DEVICE, "Reloading #ID(%s-%s)", type_str.c_str(), name_str.c_str());

	_resource_manager->reload(type, name);
	const void* new_resource = _resource_manager->get(type, name);

	if (type == RESOURCE_TYPE_SCRIPT)
	{
		_lua_environment->execute((const LuaResource*)new_resource);
	}

	logi(DEVICE, "Reloaded #ID(%s-%s)", type_str.c_str(), name_str.c_str());
}

void Device::log(const char* msg)
{
	if (_last_log)
	{
		_last_log->write(msg, strlen32(msg));
		_last_log->write("\n", 1);
		_last_log->flush();
	}
}

char _buffer[sizeof(Device)];
Device* _device = NULL;

void run(const DeviceOptions& opts)
{
	CE_ASSERT(_device == NULL, "Crown already initialized");
	console_server_globals::init();
	_device = new (_buffer) Device(opts, *console_server());
	_device->run();
	_device->~Device();
	_device = NULL;
	console_server_globals::shutdown();
}

Device* device()
{
	return crown::_device;
}

} // namespace crown
