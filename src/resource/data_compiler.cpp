/*
 * Copyright (c) 2012-2018 Daniele Bartolini and individual contributors.
 * License: https://github.com/dbartolini/crown/blob/master/LICENSE
 */

#include "config.h"
#include "core/containers/hash_map.h"
#include "core/containers/hash_set.h"
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
#include "resource/resource_id.h"
#include "resource/shader_resource.h"
#include "resource/sound_resource.h"
#include "resource/sprite_resource.h"
#include "resource/state_machine_resource.h"
#include "resource/texture_resource.h"
#include "resource/types.h"
#include "resource/unit_resource.h"
#include <algorithm>
#include <inttypes.h>
#if CROWN_PLATFORM_POSIX
#include <signal.h>
#endif

LOG_SYSTEM(DATA_COMPILER, "data_compiler")

#define CROWN_DATA_VERSIONS "data_versions.sjson"
#define CROWN_DATA_INDEX "data_index.sjson"
#define CROWN_DATA_MTIMES "data_mtimes.sjson"
#define CROWN_DATA_DEPENDENCIES "data_dependencies.sjson"

namespace crown
{
static volatile bool _quit = false;

static void notify_add_file(const char* path)
{
	TempAllocator512 ta;
	StringStream ss(ta);
	ss << "{\"type\":\"add_file\",\"path\":\"" << path << "\"}";
	console_server()->send(string_stream::c_str(ss));
}

static void notify_remove_file(const char* path)
{
	TempAllocator512 ta;
	StringStream ss(ta);
	ss << "{\"type\":\"remove_file\",\"path\":\"" << path << "\"}";
	console_server()->send(string_stream::c_str(ss));
}

static void notify_add_tree(const char* path)
{
	TempAllocator512 ta;
	StringStream ss(ta);
	ss << "{\"type\":\"add_tree\",\"path\":\"" << path << "\"}";
	console_server()->send(string_stream::c_str(ss));
}

static void notify_remove_tree(const char* path)
{
	TempAllocator512 ta;
	StringStream ss(ta);
	ss << "{\"type\":\"remove_tree\",\"path\":\"" << path << "\"}";
	console_server()->send(string_stream::c_str(ss));
}

SourceIndex::SourceIndex()
	: _paths(default_allocator())
{
}

void SourceIndex::scan_directory(FilesystemDisk& fs, const char* prefix, const char* directory)
{
	Vector<DynamicString> files(default_allocator());
	fs.list_files(directory != NULL ? directory : "", files);

	for (u32 i = 0; i < vector::size(files); ++i)
	{
		TempAllocator512 ta;
		DynamicString file_i(ta);

		if (directory != NULL)
		{
			file_i += directory;
			file_i += '/';
		}
		file_i += files[i];

		if (fs.is_directory(file_i.c_str()))
		{
			scan_directory(fs, prefix, file_i.c_str());
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

			Stat stat;
			stat = fs.stat(file_i.c_str());
			hash_map::set(_paths, resource_name, stat);

			notify_add_file(resource_name.c_str());
		}
	}
}

void SourceIndex::scan(const HashMap<DynamicString, DynamicString>& source_dirs)
{
	auto cur = hash_map::begin(source_dirs);
	auto end = hash_map::end(source_dirs);
	for (; cur != end; ++cur)
	{
		HASH_MAP_SKIP_HOLE(source_dirs, cur);

		DynamicString prefix(default_allocator());
		path::join(prefix, cur->second.c_str(), cur->first.c_str());
		FilesystemDisk fs(default_allocator());
		fs.set_prefix(prefix.c_str());
		scan_directory(fs, cur->first.c_str(), NULL);
	}
}

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

static void console_command_quit(ConsoleServer& /*cs*/, TCPSocket /*client*/, const char* /*json*/, void* /*user_data*/)
{
	_quit = true;
}

static Buffer read(FilesystemDisk& data_fs, const char* filename)
{
	Buffer buffer(default_allocator());

	// FIXME: better return NULL in Filesystem::open().
	if (data_fs.exists(filename))
	{
		File* file = data_fs.open(filename, FileOpenMode::READ);
		if (file)
		{
			u32 size = file->size();
			if (size == 0)
			{
				data_fs.close(*file);
				return buffer;
			}

			array::resize(buffer, size);
			file->read(array::begin(buffer), size);
			data_fs.close(*file);
		}
	}

	return buffer;
}

static void read_data_versions(HashMap<DynamicString, u32>& versions, FilesystemDisk& data_fs, const char* filename)
{
	Buffer json = read(data_fs, filename);

	TempAllocator512 ta;
	JsonObject object(ta);
	sjson::parse(json, object);

	auto cur = json_object::begin(object);
	auto end = json_object::end(object);
	for (; cur != end; ++cur)
	{
		JSON_OBJECT_SKIP_HOLE(object, cur);

		TempAllocator256 ta;
		DynamicString type(ta);
		type.set(cur->first.data(), cur->first.length());

		hash_map::set(versions, type, (u32)sjson::parse_int(cur->second));
	}
}

static void read_data_index(HashMap<StringId64, DynamicString>& index, FilesystemDisk& data_fs, const char* filename)
{
	Buffer json = read(data_fs, filename);

	TempAllocator512 ta;
	JsonObject object(ta);
	sjson::parse(json, object);

	auto cur = json_object::begin(object);
	auto end = json_object::end(object);
	for (; cur != end; ++cur)
	{
		JSON_OBJECT_SKIP_HOLE(object, cur);

		TempAllocator256 ta;
		StringId64 dst_name;
		DynamicString src_path(ta);

		dst_name.parse(cur->first.data());
		sjson::parse_string(cur->second, src_path);

		hash_map::set(index, dst_name, src_path);
	}
}

static void read_data_mtimes(HashMap<StringId64, u64>& mtimes, FilesystemDisk& data_fs, const char* filename)
{
	Buffer json = read(data_fs, filename);

	TempAllocator128 ta;
	JsonObject object(ta);
	sjson::parse(json, object);

	auto cur = json_object::begin(object);
	auto end = json_object::end(object);
	for (; cur != end; ++cur)
	{
		JSON_OBJECT_SKIP_HOLE(object, cur);

		TempAllocator64 ta;
		StringId64 dst_name;
		DynamicString mtime_json(ta);

		dst_name.parse(cur->first.data());
		sjson::parse_string(cur->second, mtime_json);

		u64 mtime;
		sscanf(mtime_json.c_str(), "%" SCNu64, &mtime);
		hash_map::set(mtimes, dst_name, mtime);
	}
}

static void read_data_dependencies(DataCompiler& dc, FilesystemDisk& data_fs, const char* filename)
{
	Buffer json = read(data_fs, filename);

	TempAllocator1024 ta;
	JsonObject object(ta);
	sjson::parse(json, object);

	auto cur = json_object::begin(object);
	auto end = json_object::end(object);
	for (; cur != end; ++cur)
	{
		JSON_OBJECT_SKIP_HOLE(object, cur);

		StringId64 dst_name;
		dst_name.parse(cur->first.data());

		JsonArray dependency_array(ta);
		sjson::parse_array(cur->second, dependency_array);
		for (u32 i = 0; i < array::size(dependency_array); ++i)
		{
			DynamicString src_path(ta);
			sjson::parse_string(dependency_array[i], src_path);
			if (src_path.has_prefix("//r "))
			{
				dc.add_requirement(dst_name, src_path.c_str() + 4);
			}
			else if (src_path.has_prefix("//- "))
			{
				dc.add_dependency(dst_name, src_path.c_str() + 4);
			}
			else // Assume regular dependency
			{
				dc.add_dependency(dst_name, src_path.c_str());
			}
		}
	}
}

static void write_data_index(FilesystemDisk& data_fs, const char* filename, const HashMap<StringId64, DynamicString>& index)
{
	StringStream ss(default_allocator());

	File* file = data_fs.open(filename, FileOpenMode::WRITE);
	if (file)
	{
		auto cur = hash_map::begin(index);
		auto end = hash_map::end(index);
		for (; cur != end; ++cur)
		{
			HASH_MAP_SKIP_HOLE(index, cur);

			TempAllocator256 ta;
			DynamicString str(ta);
			str.from_string_id(cur->first);
			ss << "\"" << str.c_str() << "\" = \"" << cur->second.c_str() << "\"\n";
		}

		file->write(string_stream::c_str(ss), strlen32(string_stream::c_str(ss)));
		data_fs.close(*file);
	}
}

static void write_data_versions(FilesystemDisk& data_fs, const char* filename, const HashMap<DynamicString, u32>& versions)
{
	StringStream ss(default_allocator());

	File* file = data_fs.open(filename, FileOpenMode::WRITE);
	if (file)
	{

		auto cur = hash_map::begin(versions);
		auto end = hash_map::end(versions);
		for (; cur != end; ++cur)
		{
			HASH_MAP_SKIP_HOLE(versions, cur);

			ss << cur->first.c_str() << " = " << cur->second << "\n";
		}

		file->write(string_stream::c_str(ss), strlen32(string_stream::c_str(ss)));
		data_fs.close(*file);
	}
}


static void write_data_mtimes(FilesystemDisk& data_fs, const char* filename, const HashMap<StringId64, u64>& mtimes)
{
	StringStream ss(default_allocator());

	File* file = data_fs.open(filename, FileOpenMode::WRITE);
	if (file)
	{

		auto cur = hash_map::begin(mtimes);
		auto end = hash_map::end(mtimes);
		for (; cur != end; ++cur)
		{
			HASH_MAP_SKIP_HOLE(mtimes, cur);

			TempAllocator64 ta;
			DynamicString key(ta);
			key.from_string_id(cur->first);
			ss << "\"" << key.c_str() << "\" = \"" << cur->second << "\"\n";
		}

		file->write(string_stream::c_str(ss), strlen32(string_stream::c_str(ss)));
		data_fs.close(*file);
	}
}

static void write_data_dependencies(FilesystemDisk& data_fs, const char* filename, const HashMap<StringId64, DynamicString>& index, const HashMap<StringId64, HashMap<DynamicString, u32> >& dependencies, const HashMap<StringId64, HashMap<DynamicString, u32> >& requirements)
{
	StringStream ss(default_allocator());

	File* file = data_fs.open(filename, FileOpenMode::WRITE);
	if (file)
	{
		auto cur = hash_map::begin(index);
		auto end = hash_map::end(index);
		for (; cur != end; ++cur)
		{
			HASH_MAP_SKIP_HOLE(index, cur);

			HashMap<DynamicString, u32> deps_deffault(default_allocator());
			const HashMap<DynamicString, u32>& deps = hash_map::get(dependencies, cur->first, deps_deffault);
			HashMap<DynamicString, u32> reqs_deffault(default_allocator());
			const HashMap<DynamicString, u32>& reqs = hash_map::get(requirements, cur->first, reqs_deffault);

			// Skip if data has no dependencies
			if (&deps == &deps_deffault && &reqs == &reqs_deffault)
				continue;

			TempAllocator64 ta;
			DynamicString key(ta);
			key.from_string_id(cur->first);
			ss << "\"" << key.c_str() << "\" = [\n";

			// Write all dependencies
			auto deps_cur = hash_map::begin(deps);
			auto deps_end = hash_map::end(deps);
			for (; deps_cur != deps_end; ++deps_cur)
			{
				HASH_MAP_SKIP_HOLE(deps, deps_cur);

				ss << "    \"//- " << deps_cur->first.c_str() << "\"\n";
			}

			// Write all requirements
			auto reqs_cur = hash_map::begin(reqs);
			auto reqs_end = hash_map::end(reqs);
			for (; reqs_cur != reqs_end; ++reqs_cur)
			{
				HASH_MAP_SKIP_HOLE(reqs, reqs_cur);

				ss << "    \"//r " << reqs_cur->first.c_str() << "\"\n";
			}

			ss << "]\n";
		}

		file->write(string_stream::c_str(ss), strlen32(string_stream::c_str(ss)));
		data_fs.close(*file);
	}
}

DataCompiler::DataCompiler(const DeviceOptions& opts, ConsoleServer& cs)
	: _options(&opts)
	, _console_server(&cs)
	, _source_fs(default_allocator())
	, _source_dirs(default_allocator())
	, _compilers(default_allocator())
	, _globs(default_allocator())
	, _data_index(default_allocator())
	, _data_mtimes(default_allocator())
	, _data_dependencies(default_allocator())
	, _data_requirements(default_allocator())
	, _data_versions(default_allocator())
	, _file_monitor(default_allocator())
{
	cs.register_command("compile", console_command_compile, this);
	cs.register_command("quit", console_command_quit, this);
}

DataCompiler::~DataCompiler()
{
	if (_options->_server)
		_file_monitor.stop();
}

void DataCompiler::add_file(const char* path)
{
	// Get source directory prefix
	TempAllocator512 ta;
	DynamicString source_dir(ta);
	source_dir = hash_map::get(_source_dirs, source_dir, source_dir);

	// Convert to DynamicString
	DynamicString str(ta);
	str.set(path, strlen32(path));

	// Get file status
	FilesystemDisk fs(default_allocator());
	fs.set_prefix(source_dir.c_str());
	Stat stat;
	stat = fs.stat(path);
	hash_map::set(_source_index._paths, str, stat);

	notify_add_file(path);
}

void DataCompiler::remove_file(const char* path)
{
	// Convert to DynamicString
	TempAllocator512 ta;
	DynamicString str(ta);
	str.set(path, strlen32(path));

	hash_map::remove(_source_index._paths, str);

	notify_remove_file(path);
}

void DataCompiler::add_tree(const char* path)
{
#if CROWN_PLATFORM_LINUX
	// Get source directory prefix
	TempAllocator512 ta;
	DynamicString source_dir(ta);
	source_dir = hash_map::get(_source_dirs, source_dir, source_dir);
	// Scan the directory tree.
	// See file_monitor_linux.cpp:180.
	FilesystemDisk fs(default_allocator());
	fs.set_prefix(source_dir.c_str());
	_source_index.scan_directory(fs, "", path);
#endif // CROWN_PLATFORM_LINUX

	notify_add_tree(path);
}

void DataCompiler::remove_tree(const char* path)
{
	notify_remove_tree(path);
}

void DataCompiler::map_source_dir(const char* name, const char* source_dir)
{
	TempAllocator256 ta;
	DynamicString sname(ta);
	DynamicString sdir(ta);
	sname.set(name, strlen32(name));
	sdir.set(source_dir, strlen32(source_dir));
	hash_map::set(_source_dirs, sname, sdir);
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

	deffault   = hash_map::get(_source_dirs, empty, empty);
	source_dir = hash_map::get(_source_dirs, source_name, deffault);
}

void DataCompiler::add_ignore_glob(const char* glob)
{
	TempAllocator64 ta;
	DynamicString str(ta);
	str.set(glob, strlen32(glob));
	vector::push_back(_globs, str);
}

void DataCompiler::scan_and_restore(const char* data_dir)
{
	// Scan all source directories
	s64 time_start = time::now();

	// FIXME: refactor this whole garbage
	Array<const char*> directories(default_allocator());

	auto cur = hash_map::begin(_source_dirs);
	auto end = hash_map::end(_source_dirs);
	for (; cur != end; ++cur)
	{
		HASH_MAP_SKIP_HOLE(_source_dirs, cur);

		DynamicString prefix(default_allocator());
		path::join(prefix, cur->second.c_str(), cur->first.c_str());

		char* str = (char*)default_allocator().allocate(prefix.length() + 1);
		strcpy(str, prefix.c_str());
		array::push_back(directories, (const char*)str);

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
	}

	_source_index.scan(_source_dirs);

	logi(DATA_COMPILER, "Scanned data in %.2fs", time::seconds(time::now() - time_start));

	// Restore state from previous run
	time_start = time::now();

	FilesystemDisk data_fs(default_allocator());
	data_fs.set_prefix(data_dir);

	read_data_versions(_data_versions, data_fs, CROWN_DATA_VERSIONS);
	read_data_index(_data_index, data_fs, CROWN_DATA_INDEX);
	read_data_mtimes(_data_mtimes, data_fs, CROWN_DATA_MTIMES);
	read_data_dependencies(*this, data_fs, CROWN_DATA_DEPENDENCIES);
	logi(DATA_COMPILER, "Restored state in %.2fs", time::seconds(time::now() - time_start));

	if (_options->_server)
	{
		// Start file monitor
		time_start = time::now();
		_file_monitor.start(array::size(directories)
			, array::begin(directories)
			, true
			, file_monitor_callback
			, this
			);
		logi(DATA_COMPILER, "Started file monitor in %.2fs", time::seconds(time::now() - time_start));
	}

	// Cleanup
	for (u32 i = 0, n = array::size(directories); i < n; ++i)
		default_allocator().deallocate((void*)directories[n-1-i]);
}

void DataCompiler::save(const char* data_dir)
{
	s64 time_start = time::now();

	FilesystemDisk data_fs(default_allocator());
	data_fs.set_prefix(data_dir);

	write_data_index(data_fs, CROWN_DATA_INDEX, _data_index);
	write_data_versions(data_fs, CROWN_DATA_VERSIONS, _data_versions);
	write_data_mtimes(data_fs, CROWN_DATA_MTIMES, _data_mtimes);
	write_data_dependencies(data_fs, CROWN_DATA_DEPENDENCIES, _data_index, _data_dependencies, _data_requirements);
	logi(DATA_COMPILER, "Saved state in %.2fs", time::seconds(time::now() - time_start));
}

bool DataCompiler::dependency_changed(const DynamicString& src_path, ResourceId id, u64 dst_mtime)
{
	TempAllocator1024 ta;
	DynamicString path(ta);
	destination_path(path, id);

	Stat src_stat;
	src_stat = hash_map::get(_source_index._paths, src_path, src_stat);
	if (src_stat.mtime > dst_mtime)
		return true;

	HashMap<DynamicString, u32> deffault(default_allocator());
	HashMap<DynamicString, u32>& deps = hash_map::get(_data_dependencies, id, deffault);
	auto cur = hash_map::begin(deps);
	auto end = hash_map::end(deps);
	for (; cur != end; ++cur)
	{
		HASH_MAP_SKIP_HOLE(deps, cur);

		if (src_path == cur->first)
			continue;

		if (dependency_changed(cur->first, resource_id(cur->first.c_str()), dst_mtime))
			return true;
	}

	return false;
}

bool DataCompiler::version_changed(const DynamicString& src_path, ResourceId id)
{
	const char* type = path::extension(src_path.c_str());

	if (data_version_stored(type) != data_version(type))
		return true;

	HashMap<DynamicString, u32> deffault(default_allocator());
	HashMap<DynamicString, u32>& deps = hash_map::get(_data_dependencies, id, deffault);
	auto cur = hash_map::begin(deps);
	auto end = hash_map::end(deps);
	for (; cur != end; ++cur)
	{
		HASH_MAP_SKIP_HOLE(deps, cur);

		if (src_path == cur->first)
			continue;

		if (version_changed(cur->first, resource_id(cur->first.c_str())))
			return true;
	}

	return false;
}

bool DataCompiler::should_ignore(const char* path)
{
	for (u32 ii = 0, nn = vector::size(_globs); ii < nn; ++ii)
	{
		if (wildcmp(_globs[ii].c_str(), path))
			return true;
	}

	return false;
}

bool DataCompiler::compile(const char* data_dir, const char* platform)
{
	const s64 time_start = time::now();

	FilesystemDisk data_fs(default_allocator());
	data_fs.set_prefix(data_dir);
	data_fs.create_directory("");
	data_fs.create_directory(CROWN_DATA_DIRECTORY);
	data_fs.create_directory(CROWN_TEMP_DIRECTORY);

	// Find the set of resources to be compiled
	Vector<DynamicString> to_compile(default_allocator());

	auto cur = hash_map::begin(_source_index._paths);
	auto end = hash_map::end(_source_index._paths);
	for (; cur != end; ++cur)
	{
		HASH_MAP_SKIP_HOLE(_source_index._paths, cur);

		const DynamicString& src_path = cur->first;
		const char* filename = src_path.c_str();
		const char* type = path::extension(filename);

		if (should_ignore(filename))
			continue;

		if (type == NULL || !can_compile(type))
		{
			loge(DATA_COMPILER, "Unknown resource file: '%s'", filename);
			loge(DATA_COMPILER, "Append matching pattern to " CROWN_DATAIGNORE " to ignore it");
			continue;
		}

		ResourceId id = resource_id(filename);
		TempAllocator256 ta;
		DynamicString path(ta);
		destination_path(path, id);

		u64 dst_mtime = 0;
		dst_mtime = hash_map::get(_data_mtimes, id, dst_mtime);
		Stat src_stat;
		src_stat = hash_map::get(_source_index._paths, src_path, src_stat);

		bool source_never_compiled_before    = hash_map::has(_data_index, id) == false;
		bool source_has_been_changed         = src_stat.mtime > dst_mtime;
		bool source_dependency_changed       = dependency_changed(src_path, id, dst_mtime);
		bool data_version_changed            = data_version_stored(type) != data_version(type);
		bool data_version_dependency_changed = version_changed(src_path, id);

		if (source_never_compiled_before
			|| source_has_been_changed
			|| source_dependency_changed
			|| data_version_changed
			|| data_version_dependency_changed
			)
		{
			vector::push_back(to_compile, src_path);
		}
	}

	// Sort to_compile so that ".package" resources get compiled last
	std::sort(vector::begin(to_compile), vector::end(to_compile), [](const DynamicString& resource_a, const DynamicString& resource_b)
		{
#define PACKAGE ".package"
			if ( resource_a.has_suffix(PACKAGE) && !resource_b.has_suffix(PACKAGE))
				return false;
			if (!resource_a.has_suffix(PACKAGE) &&  resource_b.has_suffix(PACKAGE))
				return true;
			return resource_a < resource_b;
#undef PACKAGE
		});

	bool success = true;

	// Compile all changed resources
	for (u32 i = 0; i < vector::size(to_compile); ++i)
	{
		const DynamicString& src_path = to_compile[i];
		const char* filename = src_path.c_str();
		const char* type = path::extension(filename);

		if (type == NULL)
			continue;

		TempAllocator1024 ta;
		DynamicString path(ta);

		// Build destination file path
		ResourceId id = resource_id(filename);
		destination_path(path, id);

		logi(DATA_COMPILER, "%s", src_path.c_str());

		// Compile data
		ResourceTypeData rtd;
		rtd.version = 0;
		rtd.compiler = NULL;

		DynamicString type_str(ta);
		type_str = type;

		rtd = hash_map::get(_compilers, type_str, rtd);
		{
			// Reset dependencies and references since data could not depend
			// anymore on any of those.
			HashMap<DynamicString, u32> dependencies_deffault(default_allocator());
			hash_map::clear(hash_map::get(_data_dependencies, id, dependencies_deffault));
			HashMap<DynamicString, u32> requirements_deffault(default_allocator());
			hash_map::clear(hash_map::get(_data_requirements, id, requirements_deffault));

			// Compile data
			Buffer output(default_allocator());
			CompileOptions opts(*this, data_fs, id, src_path, output, platform);
			success = rtd.compiler(opts) == 0;

			if (success)
			{
				File* outf = data_fs.open(path.c_str(), FileOpenMode::WRITE);
				u32 size = array::size(output);
				u32 written = outf->write(array::begin(output), size);
				data_fs.close(*outf);
				success = size == written;
			}
		}

		if (success)
		{
			hash_map::set(_data_index, id, src_path);
			hash_map::set(_data_versions, type_str, rtd.version);
			hash_map::set(_data_mtimes, id, data_fs.last_modified_time(path.c_str()));
		}
		else
		{
			loge(DATA_COMPILER, "Failed to compile data");
			break;
		}
	}

	if (success)
	{
		if (vector::size(to_compile))
			logi(DATA_COMPILER, "Compiled data in %.2fs", time::seconds(time::now() - time_start));
		else
			logi(DATA_COMPILER, "Data is up to date");
	}

	return success;
}

void DataCompiler::register_compiler(const char* type, u32 version, CompileFunction compiler)
{
	TempAllocator64 ta;
	DynamicString type_str(ta);
	type_str = type;

	CE_ASSERT(!hash_map::has(_compilers, type_str), "Type already registered");
	CE_ENSURE(NULL != compiler);

	ResourceTypeData rtd;
	rtd.version = version;
	rtd.compiler = compiler;

	hash_map::set(_compilers, type_str, rtd);
}

u32 DataCompiler::data_version(const char* type)
{
	TempAllocator64 ta;
	DynamicString type_str(ta);
	type_str = type;

	ResourceTypeData rtd;
	rtd.version = COMPILER_NOT_FOUND;
	rtd.compiler = NULL;
	return hash_map::get(_compilers, type_str, rtd).version;
}

u32 DataCompiler::data_version_stored(const char* type)
{
	TempAllocator256 ta;
	DynamicString ds(ta);
	ds = type;

	u32 version = UINT32_MAX;
	return hash_map::get(_data_versions, ds, version);
}

void DataCompiler::add_dependency_internal(HashMap<StringId64, HashMap<DynamicString, u32> >& dependencies, ResourceId id, const char* dependency)
{
	HashMap<DynamicString, u32> deps_deffault(default_allocator());
	if (hash_map::has(dependencies, id))
	{
		HashMap<DynamicString, u32>& deps = hash_map::get(dependencies, id, deps_deffault);
		TempAllocator256 ta;
		DynamicString dependency_ds(ta);
		dependency_ds = dependency;
		hash_map::set(deps, dependency_ds, 0u);
	}
	else
	{
		TempAllocator256 ta;
		DynamicString dependency_ds(ta);
		dependency_ds = dependency;
		hash_map::set(deps_deffault, dependency_ds, 0u);
		hash_map::set(dependencies, id, deps_deffault);
	}
}

void DataCompiler::add_dependency(ResourceId id, const char* dependency)
{
	add_dependency_internal(_data_dependencies, id, dependency);
}

void DataCompiler::add_requirement(ResourceId id, const char* requirement)
{
	add_dependency_internal(_data_requirements, id, requirement);
}

bool DataCompiler::can_compile(const char* type)
{
	TempAllocator64 ta;
	DynamicString type_str(ta);
	type_str = type;

	return hash_map::has(_compilers, type_str);
}

void DataCompiler::error(const char* msg, va_list args)
{
	vloge(DATA_COMPILER, msg, args);
}

void DataCompiler::file_monitor_callback(FileMonitorEvent::Enum fme, bool is_dir, const char* path, const char* path_renamed)
{
	TempAllocator512 ta;
	DynamicString source_dir(ta);
	DynamicString resource_name(ta);

	// Find source directory by matching mapped
	// directory prefix with `path`.
	auto cur = hash_map::begin(_source_dirs);
	auto end = hash_map::end(_source_dirs);
	for (; cur != end; ++cur)
	{
		HASH_MAP_SKIP_HOLE(_source_dirs, cur);

		path::join(source_dir, cur->second.c_str(), cur->first.c_str());
		if (str_has_prefix(path, source_dir.c_str()))
			break;
	}

	if (cur != end)
	{
		// All events received must refer to directories
		// mapped with map_source_dir().
		const char* filename = &path[source_dir.length()+1];
		path::join(resource_name, cur->first.c_str(), filename);

#if 0
		logi(DATA_COMPILER, "path         : %s", path);
		logi(DATA_COMPILER, "source_dir   : %s", source_dir.c_str());
		logi(DATA_COMPILER, "resource_name: %s", resource_name.c_str());
#endif

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
			{
				DynamicString resource_name_renamed(ta);
				path::join(resource_name_renamed, cur->first.c_str(), &path_renamed[source_dir.length()+1]);

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
			}
			break;

		case FileMonitorEvent::CHANGED:
			if (!is_dir)
			{
				FilesystemDisk fs(default_allocator());
				fs.set_prefix(source_dir.c_str());

				Stat stat;
				stat = fs.stat(filename);
				hash_map::set(_source_index._paths, resource_name, stat);
			}
			break;

		default:
			CE_ASSERT(false, "Unknown FileMonitorEvent: %d", fme);
			break;
		}
	}
}

void DataCompiler::file_monitor_callback(void* thiz, FileMonitorEvent::Enum fme, bool is_dir, const char* path_original, const char* path_modified)
{
	((DataCompiler*)thiz)->file_monitor_callback(fme, is_dir, path_original, path_modified);
}

int main_data_compiler(const DeviceOptions& opts)
{
#if CROWN_PLATFORM_POSIX
	struct sigaction old_SIGINT;
	struct sigaction act;
	act.sa_handler = [](int /*signum*/) { _quit = true; };
	sigemptyset(&act.sa_mask);
	act.sa_flags = 0;
	sigaction(SIGINT, NULL, &old_SIGINT);
	sigaction(SIGINT, &act, NULL);
#endif // CROWN_PLATFORM_POSIX

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

	DataCompiler* dc = CE_NEW(default_allocator(), DataCompiler)(opts, *console_server());
	dc->register_compiler("config",           RESOURCE_VERSION_CONFIG,           cor::compile);
	dc->register_compiler("font",             RESOURCE_VERSION_FONT,             ftr::compile);
	dc->register_compiler("level",            RESOURCE_VERSION_LEVEL,            lvr::compile);
	dc->register_compiler("material",         RESOURCE_VERSION_MATERIAL,         mtr::compile);
	dc->register_compiler("mesh",             RESOURCE_VERSION_MESH,             mhr::compile);
	dc->register_compiler("package",          RESOURCE_VERSION_PACKAGE,          pkr::compile);
	dc->register_compiler("physics_config",   RESOURCE_VERSION_PHYSICS_CONFIG,   pcr::compile);
	dc->register_compiler("lua",              RESOURCE_VERSION_SCRIPT,           lur::compile);
	dc->register_compiler("shader",           RESOURCE_VERSION_SHADER,           shr::compile);
	dc->register_compiler("sound",            RESOURCE_VERSION_SOUND,            sdr::compile);
	dc->register_compiler("sprite",           RESOURCE_VERSION_SPRITE,           spr::compile);
	dc->register_compiler("sprite_animation", RESOURCE_VERSION_SPRITE_ANIMATION, sar::compile);
	dc->register_compiler("state_machine",    RESOURCE_VERSION_STATE_MACHINE,    smr::compile);
	dc->register_compiler("texture",          RESOURCE_VERSION_TEXTURE,          txr::compile);
	dc->register_compiler("unit",             RESOURCE_VERSION_UNIT,             utr::compile);

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

	dc->scan_and_restore(opts._data_dir.c_str());

	bool success = true;

	if (opts._server)
	{
		while (!_quit)
		{
			console_server()->update();
			os::sleep(60);
		}
	}
	else
	{
		success = dc->compile(opts._data_dir.c_str(), opts._platform);
	}

	dc->save(opts._data_dir.c_str());

	CE_DELETE(default_allocator(), dc);
	console_server_globals::shutdown();

#if CROWN_PLATFORM_POSIX
	// Restore original handler
	sigaction(SIGINT, &old_SIGINT, NULL);
#endif // CROWN_PLATFORM_POSIX

	return success ? EXIT_SUCCESS : EXIT_FAILURE;
}

} // namespace crown
