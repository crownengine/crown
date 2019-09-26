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
#include "resource/shader_resource.h"
#include "resource/sound_resource.h"
#include "resource/sprite_resource.h"
#include "resource/state_machine_resource.h"
#include "resource/texture_resource.h"
#include "resource/types.h"
#include "resource/unit_resource.h"
#include <algorithm>

LOG_SYSTEM(DATA_COMPILER, "data_compiler")

#define CROWN_DATA_VERSIONS "data_versions.sjson"
#define CROWN_DATA_INDEX "data_index.sjson"
#define CROWN_DATA_MTIMES "data_mtimes.sjson"
#define CROWN_DATA_DEPENDENCIES "data_dependencies.sjson"

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

static bool _quit = false;

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
		if (json_object::is_hole(object, cur))
			continue;

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
		if (json_object::is_hole(object, cur))
			continue;

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
		if (json_object::is_hole(object, cur))
			continue;

		TempAllocator64 ta;
		StringId64 dst_name;
		DynamicString mtime_json(ta);

		dst_name.parse(cur->first.data());
		sjson::parse_string(cur->second, mtime_json);

		u64 mtime;
		sscanf(mtime_json.c_str(), "%lu", &mtime);
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
		if (json_object::is_hole(object, cur))
			continue;

		StringId64 dst_name;
		dst_name.parse(cur->first.data());

		JsonArray dependency_array(ta);
		sjson::parse_array(cur->second, dependency_array);
		for (u32 i = 0; i < array::size(dependency_array); ++i)
		{
			DynamicString src_path(ta);
			sjson::parse_string(dependency_array[i], src_path);
			dc.add_dependency(dst_name, src_path.c_str());
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
			if (hash_map::is_hole(index, cur))
				continue;

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
			if (hash_map::is_hole(versions, cur))
				continue;

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
			if (hash_map::is_hole(mtimes, cur))
				continue;

			TempAllocator64 ta;
			DynamicString key(ta);
			key.from_string_id(cur->first);
			ss << "\"" << key.c_str() << "\" = \"" << cur->second << "\"\n";
		}

		file->write(string_stream::c_str(ss), strlen32(string_stream::c_str(ss)));
		data_fs.close(*file);
	}
}

static void write_source_files(StringStream& ss, HashMap<DynamicString, u32> sources)
{
	auto cur = hash_map::begin(sources);
	auto end = hash_map::end(sources);
	for (; cur != end; ++cur)
	{
		if (hash_map::is_hole(sources, cur))
			continue;

		ss << "    \"" << cur->first.c_str() << "\"\n";
	}
}

static void write_data_dependencies(FilesystemDisk& data_fs, const char* filename, const HashMap<StringId64, HashMap<DynamicString, u32> >& dependencies)
{
	StringStream ss(default_allocator());

	File* file = data_fs.open(filename, FileOpenMode::WRITE);
	if (file)
	{
		auto cur = hash_map::begin(dependencies);
		auto end = hash_map::end(dependencies);
		for (; cur != end; ++cur)
		{
			if (hash_map::is_hole(dependencies, cur))
				continue;

			TempAllocator64 ta;
			DynamicString key(ta);
			key.from_string_id(cur->first);
			ss << "\"" << key.c_str() << "\" = [\n";
			write_source_files(ss, cur->second);
			ss << "]\n";
		}

		file->write(string_stream::c_str(ss), strlen32(string_stream::c_str(ss)));
		data_fs.close(*file);
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
	, _data_mtimes(default_allocator())
	, _data_dependencies(default_allocator())
	, _data_versions(default_allocator())
	, _file_monitor(default_allocator())
{
	cs.register_command("compile", console_command_compile, this);
	cs.register_command("quit", console_command_quit, this);
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
	source_dir = hash_map::get(_source_dirs, source_dir, source_dir);

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

	auto cur = hash_map::begin(_source_dirs);
	auto end = hash_map::end(_source_dirs);
	for (; cur != end; ++cur)
	{
		if (hash_map::is_hole(_source_dirs, cur))
			continue;

		DynamicString prefix(default_allocator());
		path::join(prefix, cur->second.c_str(), cur->first.c_str());
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

		scan_source_dir(cur->first.c_str(), "");
	}
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

	_file_monitor.start(hash_map::begin(_source_dirs)->second.c_str(), true, filemonitor_callback, this);
}

void DataCompiler::save(const char* data_dir)
{
	s64 time_start = time::now();

	FilesystemDisk data_fs(default_allocator());
	data_fs.set_prefix(data_dir);

	write_data_index(data_fs, CROWN_DATA_INDEX, _data_index);
	write_data_versions(data_fs, CROWN_DATA_VERSIONS, _data_versions);
	write_data_mtimes(data_fs, CROWN_DATA_MTIMES, _data_mtimes);
	write_data_dependencies(data_fs, CROWN_DATA_DEPENDENCIES, _data_dependencies);
	logi(DATA_COMPILER, "Saved state in %.2fs", time::seconds(time::now() - time_start));
}

/// Returns the resource id from @a type and @a name.
ResourceId resource_id(const char* type, u32 type_len, const char* name, u32 name_len)
{
	ResourceId id;
	id._id = StringId64(type, type_len)._id ^ StringId64(name, name_len)._id;
	return id;
}

/// Returns the resource id from @a filename.
ResourceId resource_id(const char* filename)
{
	const char* type = path::extension(filename);
	const u32 len = u32(type - filename - 1);
	return resource_id(type, strlen32(type), filename, len);
}

void destination_path(DynamicString& path, ResourceId id)
{
	TempAllocator128 ta;
	DynamicString id_hex(ta);
	id_hex.from_string_id(id);
	path::join(path, CROWN_DATA_DIRECTORY, id_hex.c_str());
}

bool DataCompiler::dependency_changed(const DynamicString& src_path, ResourceId id, u64 dst_mtime)
{
	TempAllocator1024 ta;
	DynamicString path(ta);
	destination_path(path, id);

	// Look up source path
#if 0
	DynamicString src_path_deffault(ta);
	DynamicString& src_path = hash_map::get(_data_index, id, src_path_deffault);
	CE_ENSURE(!src_path.empty());
#endif

	DynamicString source_dir(ta);
	this->source_dir(src_path.c_str(), source_dir);

	FilesystemDisk source_fs(ta);
	source_fs.set_prefix(source_dir.c_str());

	u64 src_mtime = source_fs.last_modified_time(src_path.c_str());
	if (src_mtime > dst_mtime)
	{
		// printf("changed: %s. src_mtime = %lu, dst_mtime = %lu\n", src_path.c_str(), src_mtime, dst_mtime);
		return true;
	}

	HashMap<DynamicString, u32> deffault(default_allocator());
	HashMap<DynamicString, u32>& deps = hash_map::get(_data_dependencies, id, deffault);
	auto cur = hash_map::begin(deps);
	auto end = hash_map::end(deps);
	for (; cur != end; ++cur)
	{
		if (hash_map::is_hole(deps, cur))
			continue;

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
		if (hash_map::is_hole(deps, cur))
			continue;

		if (src_path == cur->first)
			continue;

		if (version_changed(cur->first, resource_id(cur->first.c_str())))
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

	if (!data_fs.exists(CROWN_DATA_DIRECTORY))
		data_fs.create_directory(CROWN_DATA_DIRECTORY);

	if (!data_fs.exists(CROWN_TEMP_DIRECTORY))
		data_fs.create_directory(CROWN_TEMP_DIRECTORY);

	// Find the set of resources to be compiled
	Vector<DynamicString> to_compile(default_allocator());
	for (u32 i = 0; i < vector::size(_files); ++i)
	{
		const DynamicString& src_path = _files[i];
		const char* filename = src_path.c_str();

		const char* type = path::extension(filename);
		ResourceId id = resource_id(filename);
		TempAllocator256 ta;
		DynamicString path(ta);
		destination_path(path, id);

		u64 dst_mtime = 0;
		dst_mtime = hash_map::get(_data_mtimes, id, dst_mtime);

		DynamicString source_dir(ta);
		this->source_dir(filename, source_dir);
		FilesystemDisk source_fs(ta);
		source_fs.set_prefix(source_dir.c_str());

		if (data_fs.exists(path.c_str()) == false
			|| source_fs.last_modified_time(filename) > dst_mtime
			|| dependency_changed(src_path, id, dst_mtime)
			|| data_version_stored(type) != data_version(type)
			|| version_changed(src_path, id)
			)
		{
			vector::push_back(to_compile, _files[i]);
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

	bool success = false;

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

		if (!can_compile(type))
		{
			loge(DATA_COMPILER, "Unknown resource type: '%s'", type);
			loge(DATA_COMPILER, "Append extension to " CROWN_DATAIGNORE " to ignore the type");
			success = false;
			break;
		}

		// Compile data
		ResourceTypeData rtd;
		rtd.version = 0;
		rtd.compiler = NULL;

		DynamicString type_str(ta);
		type_str = type;

		rtd = hash_map::get(_compilers, type_str, rtd);
		{
			Buffer output(default_allocator());
			array::reserve(output, 4*1024*1024);

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

	if (success && vector::size(to_compile))
	{
		logi(DATA_COMPILER, "Compiled data in %.2fs", time::seconds(time::now() - time_start));
	}
	else
	{
		logi(DATA_COMPILER, "Data is up to date");
		success = true;
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

void DataCompiler::add_dependency(ResourceId id, const char* dependency)
{
	HashMap<DynamicString, u32> deps_deffault(default_allocator());
	if (hash_map::has(_data_dependencies, id))
	{
		HashMap<DynamicString, u32>& deps = hash_map::get(_data_dependencies, id, deps_deffault);
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
		hash_map::set(_data_dependencies, id, deps_deffault);
	}
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

void DataCompiler::filemonitor_callback(FileMonitorEvent::Enum fme, bool is_dir, const char* path, const char* path_renamed)
{
	TempAllocator512 ta;
	DynamicString resource_name(ta);
	DynamicString resource_name_renamed(ta);
	DynamicString source_dir(ta);

	source_dir            = hash_map::get(_source_dirs, source_dir, source_dir);
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

	return success ? EXIT_SUCCESS : EXIT_FAILURE;
}

} // namespace crown
