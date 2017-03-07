/*
 * Copyright (c) 2012-2017 Daniele Bartolini and individual contributors.
 * License: https://github.com/taylor001/crown/blob/master/LICENSE
 */

#include "allocator.h"
#include "compile_options.h"
#include "config.h"
#include "console_server.h"
#include "data_compiler.h"
#include "dynamic_string.h"
#include "file.h"
#include "filesystem_disk.h"
#include "log.h"
#include "map.h"
#include "os.h"
#include "path.h"
#include "sjson.h"
#include "sort_map.h"
#include "string_stream.h"
#include "temp_allocator.h"
#include "vector.h"
#include <setjmp.h>

#include "resource_types.h"
#include "config_resource.h"
#include "font_resource.h"
#include "lua_resource.h"
#include "level_resource.h"
#include "mesh_resource.h"
#include "material_resource.h"
#include "physics_resource.h"
#include "package_resource.h"
#include "sound_resource.h"
#include "shader_resource.h"
#include "sprite_resource.h"
#include "texture_resource.h"
#include "unit_resource.h"
#include "device_options.h"
#include "console_api.h"
#include "json_object.h"

namespace crown
{
class LineReader
{
	const char* _str;
	const u32 _len;
	u32 _pos;

public:

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

	logi("Compiling '%s'", id.c_str());
	bool succ = ((DataCompiler*)user_data)->compile(data_dir.c_str(), platform.c_str());

	if (succ)
		logi("Compiled '%s'", id.c_str());
	else
		loge("Error while compiling '%s'", id.c_str());

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

bool DataCompiler::can_compile(StringId64 type)
{
	return sort_map::has(_compilers, type);
}

void DataCompiler::compile(StringId64 type, const char* path, CompileOptions& opts)
{
	CE_ASSERT(sort_map::has(_compilers, type), "Compiler not found");

	sort_map::get(_compilers, type, ResourceTypeData()).compiler(path, opts);
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

bool DataCompiler::compile(FilesystemDisk& bundle_fs, const char* type, const char* name, const char* platform)
{
	TempAllocator1024 ta;
	DynamicString path(ta);
	DynamicString src_path(ta);
	DynamicString type_str(ta);
	DynamicString name_str(ta);
	DynamicString dst_path(ta);

	StringId64 _type(type);
	StringId64 _name(name);

	// Build source file path
	src_path += name;
	src_path += '.';
	src_path += type;

	// Build compiled file path
	_type.to_string(type_str);
	_name.to_string(name_str);

	// Build destination file path
	dst_path += type_str;
	dst_path += '-';
	dst_path += name_str;

	path::join(path, CROWN_DATA_DIRECTORY, dst_path.c_str());

	logi("%s <= %s", dst_path.c_str(), src_path.c_str());

	if (!can_compile(_type))
	{
		loge("Unknown resource type: '%s'", type);
		return false;
	}

	bool success = true;
	jmp_buf buf;

	Buffer output(default_allocator());
	array::reserve(output, 4*1024*1024);

	if (!setjmp(buf))
	{
		CompileOptions opts(*this, bundle_fs, output, platform, &buf);

		compile(_type, src_path.c_str(), opts);

		File* outf = bundle_fs.open(path.c_str(), FileOpenMode::WRITE);
		u32 size = array::size(output);
		u32 written = outf->write(array::begin(output), size);
		bundle_fs.close(*outf);

		success = size == written;
	}
	else
	{
		success = false;
	}

	return success;
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
	// Scan all source directories
	auto cur = map::begin(_source_dirs);
	auto end = map::end(_source_dirs);

	for (; cur != end; ++cur)
	{
		DynamicString prefix(default_allocator());
		prefix += cur->pair.second.c_str();
		prefix += '/';
		prefix += cur->pair.first.c_str();
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

	_file_monitor.start(map::begin(_source_dirs)->pair.second.c_str(), true, filemonitor_callback, this);
}

bool DataCompiler::compile(const char* bundle_dir, const char* platform)
{
	// Create bundle dir if necessary
	FilesystemDisk bundle_fs(default_allocator());
	bundle_fs.set_prefix(bundle_dir);
	bundle_fs.create_directory("");

	if (!bundle_fs.exists(CROWN_DATA_DIRECTORY))
		bundle_fs.create_directory(CROWN_DATA_DIRECTORY);

	if (!bundle_fs.exists(CROWN_TEMP_DIRECTORY))
		bundle_fs.create_directory(CROWN_TEMP_DIRECTORY);

	std::sort(vector::begin(_files), vector::end(_files));

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

		if (!compile(bundle_fs, type, name, platform))
			return false;
	}

	return true;
}

void DataCompiler::register_compiler(StringId64 type, u32 version, CompileFunction compiler)
{
	CE_ASSERT(!sort_map::has(_compilers, type), "Type already registered");
	CE_ENSURE(NULL != compiler);

	ResourceTypeData rtd;
	rtd.version = version;
	rtd.compiler = compiler;

	sort_map::set(_compilers, type, rtd);
	sort_map::sort(_compilers);
}

u32 DataCompiler::version(StringId64 type)
{
	CE_ASSERT(sort_map::has(_compilers, type), "Compiler not found");

	return sort_map::get(_compilers, type, ResourceTypeData()).version;
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

int main_data_compiler(int argc, char** argv)
{
	InitMemoryGlobals m;
	CE_UNUSED(m);

	DeviceOptions opts(default_allocator(), argc, (const char**)argv);
	if (opts.parse() == EXIT_FAILURE)
		return EXIT_FAILURE;

	console_server_globals::init();
	load_console_api(*console_server());

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
	dc->register_compiler(RESOURCE_TYPE_TEXTURE,          RESOURCE_VERSION_TEXTURE,          txr::compile);
	dc->register_compiler(RESOURCE_TYPE_UNIT,             RESOURCE_VERSION_UNIT,             utr::compile);

	// Add ignore globs
	dc->add_ignore_glob("*.tmp");
	dc->add_ignore_glob("*.wav");
	dc->add_ignore_glob("*.ogg");
	dc->add_ignore_glob("*.png");
	dc->add_ignore_glob("*.tga");
	dc->add_ignore_glob("*.dds");
	dc->add_ignore_glob("*.ktx");
	dc->add_ignore_glob("*.pvr");
	dc->add_ignore_glob("*.swn"); // VIM swap file.
	dc->add_ignore_glob("*.swo"); // VIM swap file.
	dc->add_ignore_glob("*.swp"); // VIM swap file.
	dc->add_ignore_glob("*.bak");
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
