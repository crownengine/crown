/*
 * Copyright (c) 2012-2018 Daniele Bartolini and individual contributors.
 * License: https://github.com/dbartolini/crown/blob/master/LICENSE
 */

#include "config.h"
#include "core/containers/hash_map.h"
#include "core/containers/map.h"
#include "core/containers/vector.h"
#include "core/filesystem/file.h"
#include "core/filesystem/filesystem_disk.h"
#include "core/filesystem/path.h"
#include "core/json/json_object.h"
#include "core/json/sjson.h"
#include "core/memory/allocator.h"
#include "core/memory/temp_allocator.h"
#include "core/os.h"
#include "core/strings/dynamic_string.h"
#include "core/strings/string_stream.h"
#include "core/time.h"
#include "device/console_server.h"
#include "device/device_options.h"
#include "device/log.h"
#include "resource/compile_options.h"
#include "resource/config_resource.h"
#include "resource/data_compiler.h"
#include "resource/font_resource.h"
#include "resource/level_resource.h"
#include "resource/lua_resource.h"
#include "resource/material_resource.h"
#include "resource/mesh_resource.h"
#include "resource/package_resource.h"
#include "resource/physics_resource.h"
#include "resource/shader_resource.h"
#include "resource/sound_resource.h"
#include "resource/sprite_resource.h"
#include "resource/state_machine_resource.h"
#include "resource/texture_resource.h"
#include "resource/types.h"
#include "resource/unit_resource.h"
#include <algorithm>

LOG_SYSTEM(DATA_COMPILER, "data_compiler")

namespace crown
{
struct LineReader
{
	const char* _str;
	const u32 _len;
	u32 _pos;

	LineReader(const char* str)
		: _str(str)
		, _len(strlen32(str))
		, _pos(0)
	{
	}

	void read_line(DynamicString& line)
	{
		const char* s  = &_str[_pos];
		const char* nl = strnl(s);
		_pos += u32(nl - s);
		line.set(s, u32(nl - s));
	}

	bool eof()
	{
		return _str[_pos] == '\0';
	}
};

static void console_command_compile(ConsoleServer& cs, TCPSocket client, const char* json, void* user_data)
{
	TempAllocator4096 ta;
	JsonObject obj(ta);
	DynamicString id(ta);
	DynamicString data_dir(ta);
	DynamicString platform(ta);

	sjson::parse(json, obj);
	sjson::parse_string(obj["id"], id);
	sjson::parse_string(obj["data_dir"], data_dir);
	sjson::parse_string(obj["platform"], platform);

	{
		TempAllocator512 ta;
		StringStream ss(ta);
		ss << "{\"type\":\"compile\",\"id\":\"" << id.c_str() << "\",\"start\":true}";
		cs.send(client, string_stream::c_str(ss));
	}

	bool succ = ((DataCompiler*)user_data)->compile(data_dir.c_str(), platform.c_str());

	{
		TempAllocator512 ta;
		StringStream ss(ta);
		ss << "{\"type\":\"compile\",\"id\":\"" << id.c_str() << "\",\"success\":" << (succ ? "true" : "false") << "}";
		cs.send(client, string_stream::c_str(ss));
	}
}

DataCompiler::DataCompiler(ConsoleServer& cs)
	: _console_server(&cs)
	, _source_fs(default_allocator())
	, _source_dirs(default_allocator())
	, _compilers(default_allocator())
	, _files(default_allocator())
	, _globs(default_allocator())
	, _data_index(default_allocator())
	, _file_monitor(default_allocator())
{
	cs.register_command("compile", console_command_compile, this);
}

DataCompiler::~DataCompiler()
{
	_file_monitor.stop();
}

void DataCompiler::add_file(const char* path)
{
	for (u32 gg = 0; gg < vector::size(_globs); ++gg)
	{
		if (wildcmp(_globs[gg].c_str(), path))
			return;
	}

	TempAllocator512 ta;
	DynamicString str(ta);
	str.set(path, strlen32(path));
	vector::push_back(_files, str);

	StringStream ss(ta);
	ss << "{\"type\":\"add_file\",\"path\":\"" << str.c_str() << "\"}";
	_console_server->send(string_stream::c_str(ss));
}

void DataCompiler::add_tree(const char* path)
{
	TempAllocator512 ta;
	DynamicString source_dir(ta);
	source_dir = map::get(_source_dirs, source_dir, source_dir);

	_source_fs.set_prefix(source_dir.c_str());
	DataCompiler::scan_source_dir(source_dir.c_str(), path);

	StringStream ss(ta);
	ss << "{\"type\":\"add_tree\",\"path\":\"" << path << "\"}";
	_console_server->send(string_stream::c_str(ss));
}

void DataCompiler::remove_file(const char* path)
{
	for (u32 i = 0; i < vector::size(_files); ++i)
	{
		if (_files[i] == path)
		{
			_files[i] = _files[vector::size(_files) - 1];
			vector::pop_back(_files);

			TempAllocator512 ta;
			StringStream ss(ta);
			ss << "{\"type\":\"remove_file\",\"path\":\"" << path << "\"}";
			_console_server->send(string_stream::c_str(ss));
			return;
		}
	}
}

void DataCompiler::remove_tree(const char* path)
{
	TempAllocator512 ta;
	StringStream ss(ta);
	ss << "{\"type\":\"remove_tree\",\"path\":\"" << path << "\"}";
	_console_server->send(string_stream::c_str(ss));

	for (u32 i = 0; i < vector::size(_files);)
	{
		if (_files[i].has_prefix(path))
		{
			TempAllocator512 ta;
			StringStream ss(ta);
			ss << "{\"type\":\"remove_file\",\"path\":\"" << _files[i].c_str() << "\"}";
			_console_server->send(string_stream::c_str(ss));

			_files[i] = _files[vector::size(_files) - 1];
			vector::pop_back(_files);
			continue;
		}

		++i;
	}
}

void DataCompiler::scan_source_dir(const char* prefix, const char* cur_dir)
{
	Vector<DynamicString> my_files(default_allocator());
	_source_fs.list_files(cur_dir, my_files);

	for (u32 i = 0; i < vector::size(my_files); ++i)
	{
		TempAllocator512 ta;
		DynamicString file_i(ta);

		if (strcmp(cur_dir, "") != 0)
		{
			file_i += cur_dir;
			file_i += '/';
		}
		file_i += my_files[i];

		if (_source_fs.is_directory(file_i.c_str()))
		{
			DataCompiler::scan_source_dir(prefix, file_i.c_str());
		}
		else // Assume a regular file
		{
			DynamicString resource_name(ta);
			if (strcmp(prefix, "") != 0)
			{
				resource_name += prefix;
				resource_name += '/';
			}
			resource_name += file_i;
			add_file(resource_name.c_str());
		}
	}
}

void DataCompiler::map_source_dir(const char* name, const char* source_dir)
{
	TempAllocator256 ta;
	DynamicString sname(ta);
	DynamicString sdir(ta);
	sname.set(name, strlen32(name));
	sdir.set(source_dir, strlen32(source_dir));
	map::set(_source_dirs, sname, sdir);
}

void DataCompiler::source_dir(const char* resource_name, DynamicString& source_dir)
{
	const char* slash = strchr(resource_name, '/');

	TempAllocator256 ta;
	DynamicString source_name(ta);

	if (slash != NULL)
		source_name.set(resource_name, u32(slash - resource_name));
	else
		source_name.set("", 0);

	DynamicString deffault(ta);
	DynamicString empty(ta);
	empty = "";

	deffault   = map::get(_source_dirs, empty, empty);
	source_dir = map::get(_source_dirs, source_name, deffault);
}

void DataCompiler::add_ignore_glob(const char* glob)
{
	TempAllocator64 ta;
	DynamicString str(ta);
	str.set(glob, strlen32(glob));
	vector::push_back(_globs, str);
}

void DataCompiler::scan()
{
	const s64 time_start = time::now();

	// Scan all source directories
	auto cur = map::begin(_source_dirs);
	auto end = map::end(_source_dirs);

	for (; cur != end; ++cur)
	{
		DynamicString prefix(default_allocator());
		path::join(prefix, cur->pair.second.c_str(), cur->pair.first.c_str());
		_source_fs.set_prefix(prefix.c_str());

		if (_source_fs.exists(CROWN_DATAIGNORE))
		{
			File& file = *_source_fs.open(CROWN_DATAIGNORE, FileOpenMode::READ);
			const u32 size = file.size();
			char* data = (char*)default_allocator().allocate(size + 1);
			file.read(data, size);
			data[size] = '\0';
			_source_fs.close(file);

			LineReader lr(data);

			while (!lr.eof())
			{
				TempAllocator512 ta;
				DynamicString line(ta);
				lr.read_line(line);

				line.trim();

				if (line.empty() || line.has_prefix("#"))
					continue;

				add_ignore_glob(line.c_str());
			}

			default_allocator().deallocate(data);
		}

		scan_source_dir(cur->pair.first.c_str(), "");
	}

	logi(DATA_COMPILER, "Scanned data in %.2fs", time::seconds(time::now() - time_start));
	_file_monitor.start(map::begin(_source_dirs)->pair.second.c_str(), true, filemonitor_callback, this);
}

bool DataCompiler::compile(const char* data_dir, const char* platform)
{
	const s64 time_start = time::now();

	FilesystemDisk data_filesystem(default_allocator());
	data_filesystem.set_prefix(data_dir);
	data_filesystem.create_directory("");

	if (!data_filesystem.exists(CROWN_DATA_DIRECTORY))
		data_filesystem.create_directory(CROWN_DATA_DIRECTORY);

	if (!data_filesystem.exists(CROWN_TEMP_DIRECTORY))
		data_filesystem.create_directory(CROWN_TEMP_DIRECTORY);

	std::sort(vector::begin(_files), vector::end(_files), [](const DynamicString& resource_a, const DynamicString& resource_b)
		{
#define PACKAGE ".package"
			if ( resource_a.has_suffix(PACKAGE) && !resource_b.has_suffix(PACKAGE))
				return false;
			if (!resource_a.has_suffix(PACKAGE) &&  resource_b.has_suffix(PACKAGE))
				return true;
			return resource_a < resource_b;
#undef PACKAGE
		});

	bool success = false;

	// Compile all changed resources
	for (u32 i = 0; i < vector::size(_files); ++i)
	{
		const char* filename = _files[i].c_str();
		const char* type = path::extension(filename);

		if (type == NULL)
			continue;

		char name[256];
		const u32 size = u32(type - filename - 1);
		strncpy(name, filename, size);
		name[size] = '\0';

		TempAllocator1024 ta;
		DynamicString path(ta);
		DynamicString src_path(ta);
		DynamicString dst_path(ta);

		StringId64 _type(type);
		StringId64 _name(name);

		// Build source file path
		src_path += name;
		src_path += '.';
		src_path += type;

		// Build destination file path
		StringId64 mix;
		mix._id = _type._id ^ _name._id;
		mix.to_string(dst_path);

		path::join(path, CROWN_DATA_DIRECTORY, dst_path.c_str());

		logi(DATA_COMPILER, "%s", src_path.c_str());

		if (!can_compile(_type))
		{
			loge(DATA_COMPILER, "Unknown resource type: '%s'", type);
			loge(DATA_COMPILER, "Append extension to " CROWN_DATAIGNORE " to ignore the type");
			success = false;
			break;
		}

		Buffer output(default_allocator());
		array::reserve(output, 4*1024*1024);

		if (!setjmp(_jmpbuf))
		{
			CompileOptions opts(*this, data_filesystem, src_path, output, platform);

			hash_map::get(_compilers, _type, ResourceTypeData()).compiler(opts);

			File* outf = data_filesystem.open(path.c_str(), FileOpenMode::WRITE);
			u32 size = array::size(output);
			u32 written = outf->write(array::begin(output), size);
			data_filesystem.close(*outf);

			success = size == written;
		}
		else
		{
			success = false;
		}

		if (success)
		{
			if (!map::has(_data_index, dst_path))
				map::set(_data_index, dst_path, src_path);
		}
		else
		{
			loge(DATA_COMPILER, "Failed to compile data");
			break;
		}
	}

	// Write data index
	{
		File* file = data_filesystem.open("data_index.sjson", FileOpenMode::WRITE);
		if (file)
		{
			StringStream ss(default_allocator());

			auto cur = map::begin(_data_index);
			auto end = map::end(_data_index);
			for (; cur != end; ++cur)
			{
				ss << "\"" << cur->pair.first.c_str() << "\" = \"" << cur->pair.second.c_str() << "\"\n";
			}

			file->write(string_stream::c_str(ss), strlen32(string_stream::c_str(ss)));
			data_filesystem.close(*file);
		}
	}

	if (success)
		logi(DATA_COMPILER, "Compiled data in %.2fs", time::seconds(time::now() - time_start));

	return success;
}

void DataCompiler::register_compiler(StringId64 type, u32 version, CompileFunction compiler)
{
	CE_ASSERT(!hash_map::has(_compilers, type), "Type already registered");
	CE_ENSURE(NULL != compiler);

	ResourceTypeData rtd;
	rtd.version = version;
	rtd.compiler = compiler;

	hash_map::set(_compilers, type, rtd);
}

u32 DataCompiler::version(StringId64 type)
{
	ResourceTypeData rtd;
	rtd.version = COMPILER_NOT_FOUND;
	rtd.compiler = NULL;

	return hash_map::get(_compilers, type, rtd).version;
}

bool DataCompiler::can_compile(StringId64 type)
{
	return hash_map::has(_compilers, type);
}

void DataCompiler::error(const char* msg, va_list args)
{
	vloge(DATA_COMPILER, msg, args);
	longjmp(_jmpbuf, 1);
}

void DataCompiler::filemonitor_callback(FileMonitorEvent::Enum fme, bool is_dir, const char* path, const char* path_renamed)
{
	TempAllocator512 ta;
	DynamicString resource_name(ta);
	DynamicString resource_name_renamed(ta);
	DynamicString source_dir(ta);

	source_dir            = map::get(_source_dirs, source_dir, source_dir);
	resource_name         = &path[source_dir.length()+1]; // FIXME: add path::relative()
	resource_name_renamed = path_renamed ? &path_renamed[source_dir.length()+1] : "";

	switch (fme)
	{
	case FileMonitorEvent::CREATED:
		if (!is_dir)
			add_file(resource_name.c_str());
		else
			add_tree(resource_name.c_str());
		break;

	case FileMonitorEvent::DELETED:
		if (!is_dir)
			remove_file(resource_name.c_str());
		else
			remove_tree(resource_name.c_str());
		break;

	case FileMonitorEvent::RENAMED:
		if (!is_dir)
		{
			remove_file(resource_name.c_str());
			add_file(resource_name_renamed.c_str());
		}
		else
		{
			remove_tree(resource_name.c_str());
			add_tree(resource_name_renamed.c_str());
		}
		break;

	case FileMonitorEvent::CHANGED:
		break;

	default:
		CE_ASSERT(false, "Unknown FileMonitorEvent: %d", fme);
		break;
	}
}

void DataCompiler::filemonitor_callback(void* thiz, FileMonitorEvent::Enum fme, bool is_dir, const char* path_original, const char* path_modified)
{
	((DataCompiler*)thiz)->filemonitor_callback(fme, is_dir, path_original, path_modified);
}

struct InitMemoryGlobals
{
	InitMemoryGlobals()
	{
		crown::memory_globals::init();
	}

	~InitMemoryGlobals()
	{
		crown::memory_globals::shutdown();
	}
};

int main_data_compiler(const DeviceOptions& opts)
{
	console_server_globals::init();
	console_server()->listen(CROWN_DEFAULT_COMPILER_PORT, opts._wait_console);

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
	namespace smr = state_machine_internal;
	namespace spr = sprite_resource_internal;
	namespace txr = texture_resource_internal;
	namespace utr = unit_resource_internal;

	DataCompiler* dc = CE_NEW(default_allocator(), DataCompiler)(*console_server());
	dc->register_compiler(RESOURCE_TYPE_CONFIG,           RESOURCE_VERSION_CONFIG,           cor::compile);
	dc->register_compiler(RESOURCE_TYPE_FONT,             RESOURCE_VERSION_FONT,             ftr::compile);
	dc->register_compiler(RESOURCE_TYPE_LEVEL,            RESOURCE_VERSION_LEVEL,            lvr::compile);
	dc->register_compiler(RESOURCE_TYPE_MATERIAL,         RESOURCE_VERSION_MATERIAL,         mtr::compile);
	dc->register_compiler(RESOURCE_TYPE_MESH,             RESOURCE_VERSION_MESH,             mhr::compile);
	dc->register_compiler(RESOURCE_TYPE_PACKAGE,          RESOURCE_VERSION_PACKAGE,          pkr::compile);
	dc->register_compiler(RESOURCE_TYPE_PHYSICS,          RESOURCE_VERSION_PHYSICS,          phr::compile);
	dc->register_compiler(RESOURCE_TYPE_PHYSICS_CONFIG,   RESOURCE_VERSION_PHYSICS_CONFIG,   pcr::compile);
	dc->register_compiler(RESOURCE_TYPE_SCRIPT,           RESOURCE_VERSION_SCRIPT,           lur::compile);
	dc->register_compiler(RESOURCE_TYPE_SHADER,           RESOURCE_VERSION_SHADER,           shr::compile);
	dc->register_compiler(RESOURCE_TYPE_SOUND,            RESOURCE_VERSION_SOUND,            sdr::compile);
	dc->register_compiler(RESOURCE_TYPE_SPRITE,           RESOURCE_VERSION_SPRITE,           spr::compile);
	dc->register_compiler(RESOURCE_TYPE_SPRITE_ANIMATION, RESOURCE_VERSION_SPRITE_ANIMATION, sar::compile);
	dc->register_compiler(RESOURCE_TYPE_STATE_MACHINE,    RESOURCE_VERSION_STATE_MACHINE,    smr::compile);
	dc->register_compiler(RESOURCE_TYPE_TEXTURE,          RESOURCE_VERSION_TEXTURE,          txr::compile);
	dc->register_compiler(RESOURCE_TYPE_UNIT,             RESOURCE_VERSION_UNIT,             utr::compile);

	// Add ignore globs
	dc->add_ignore_glob("*.bak");
	dc->add_ignore_glob("*.dds");
	dc->add_ignore_glob("*.importer_settings");
	dc->add_ignore_glob("*.ktx");
	dc->add_ignore_glob("*.ogg");
	dc->add_ignore_glob("*.png");
	dc->add_ignore_glob("*.pvr");
	dc->add_ignore_glob("*.swn"); // VIM swap file.
	dc->add_ignore_glob("*.swo"); // VIM swap file.
	dc->add_ignore_glob("*.swp"); // VIM swap file.
	dc->add_ignore_glob("*.tga");
	dc->add_ignore_glob("*.tmp");
	dc->add_ignore_glob("*.wav");
	dc->add_ignore_glob("*~");
	dc->add_ignore_glob(".*");

	dc->map_source_dir("", opts._source_dir.c_str());

	if (opts._map_source_dir_name)
	{
		dc->map_source_dir(opts._map_source_dir_name
			, opts._map_source_dir_prefix.c_str()
			);
	}

	dc->scan();

	bool success = true;

	if (opts._server)
	{
		while (true)
		{
			console_server()->update();
			os::sleep(60);
		}
	}
	else
	{
		success = dc->compile(opts._data_dir.c_str(), opts._platform);
	}

	CE_DELETE(default_allocator(), dc);
	console_server_globals::shutdown();

	return success ? EXIT_SUCCESS : EXIT_FAILURE;
}

} // namespace crown
