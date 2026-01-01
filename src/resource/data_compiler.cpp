/*
 * Copyright (c) 2012-2026 Daniele Bartolini et al.
 * SPDX-License-Identifier: MIT
 */

#include "config.h"

#if CROWN_CAN_COMPILE
#include "core/containers/hash_map.inl"
#include "core/containers/hash_set.inl"
#include "core/containers/vector.inl"
#include "core/filesystem/file.h"
#include "core/filesystem/file_buffer.inl"
#include "core/filesystem/filesystem_disk.h"
#include "core/filesystem/path.h"
#include "core/guid.inl"
#include "core/json/json_object.inl"
#include "core/json/sjson.h"
#include "core/memory/allocator.h"
#include "core/memory/temp_allocator.inl"
#include "core/option.inl"
#include "core/os.h"
#include "core/profiler.h"
#include "core/strings/dynamic_string.inl"
#include "core/strings/string.h"
#include "core/strings/string_id.inl"
#include "core/strings/string_stream.inl"
#include "core/time.h"
#include "device/console_server.h"
#include "device/device_options.h"
#include "device/log.h"
#include "resource/compile_options.inl"
#include "resource/data_compiler.h"
#include "resource/mesh.h"
#include "resource/resource_id.inl"
#include "resource/types.h"
#include <algorithm>
#include <inttypes.h>
#if CROWN_PLATFORM_WINDOWS
	#ifndef WIN32_LEAN_AND_MEAN
		#define WIN32_LEAN_AND_MEAN
	#endif
	#include <windows.h>
#else
	#include <signal.h>
#endif

LOG_SYSTEM(DATA_COMPILER, "data_compiler")

#define CROWN_DATA_VERSIONS "data_versions.sjson"
#define CROWN_DATA_INDEX "data_index.sjson"
#define CROWN_DATA_MTIMES "data_mtimes.sjson"
#define CROWN_DATA_DEPENDENCIES "data_dependencies.sjson"

namespace crown
{
struct PlatformInfo
{
	const char *name;
	Platform::Enum type;
};

static const PlatformInfo platform_info[] =
{
	{ "android",       Platform::ANDROID       },
	{ "android-arm64", Platform::ANDROID_ARM64 },
	{ "html5",         Platform::HTML5         },
	{ "linux",         Platform::LINUX         },
	{ "windows",       Platform::WINDOWS       }
};
CE_STATIC_ASSERT(countof(platform_info) == Platform::COUNT);

static volatile bool _quit = false;

static void notify_add_file(const char *path, Stat &st)
{
	TempAllocator512 ta;
	StringStream ss(ta);
	ss << "{\"type\":\"add_file\"";
	ss << ",\"path\":\"" << path << "\"";
	ss << ",\"size\":\"" << st.size << "\"";
	ss << ",\"mtime\":\"" << st.mtime << "\"";
	ss << "}";
	console_server()->broadcast(string_stream::c_str(ss));
}

static void notify_remove_file(const char *path)
{
	TempAllocator512 ta;
	StringStream ss(ta);
	ss << "{\"type\":\"remove_file\",\"path\":\"" << path << "\"}";
	console_server()->broadcast(string_stream::c_str(ss));
}

static void notify_add_tree(const char *path)
{
	TempAllocator512 ta;
	StringStream ss(ta);
	ss << "{\"type\":\"add_tree\",\"path\":\"" << path << "\"}";
	console_server()->broadcast(string_stream::c_str(ss));
}

static void notify_remove_tree(const char *path)
{
	TempAllocator512 ta;
	StringStream ss(ta);
	ss << "{\"type\":\"remove_tree\",\"path\":\"" << path << "\"}";
	console_server()->broadcast(string_stream::c_str(ss));
}

static void notify_change_file(const char *path, Stat &st)
{
	TempAllocator512 ta;
	StringStream ss(ta);
	ss << "{\"type\":\"change_file\"";
	ss << ",\"path\":\"" << path << "\"";
	ss << ",\"size\":\"" << st.size << "\"";
	ss << ",\"mtime\":\"" << st.mtime << "\"";
	ss << "}";
	console_server()->broadcast(string_stream::c_str(ss));
}

SourceIndex::SourceIndex()
	: _paths(default_allocator())
{
}

void SourceIndex::scan_directory(FilesystemDisk &fs, const char *prefix, const char *directory)
{
	Vector<DynamicString> files(default_allocator());
	fs.list_files(directory != NULL ? directory : "", files);

	for (u32 i = 0; i < vector::size(files); ++i) {
		TempAllocator512 ta;
		DynamicString file_i(ta);

		if (directory != NULL) {
			file_i += directory;
			file_i += '/';
		}
		file_i += files[i];

		if (fs.is_directory(file_i.c_str())) {
			DynamicString directory_name(ta);
			if (strcmp(prefix, "") != 0) {
				directory_name += prefix;
				directory_name += '/';
			}
			directory_name += file_i;
			notify_add_tree(directory_name.c_str());

			scan_directory(fs, prefix, file_i.c_str());
		} else { // Assume a regular file
			DynamicString resource_name(ta);
			if (strcmp(prefix, "") != 0) {
				resource_name += prefix;
				resource_name += '/';
			}
			resource_name += file_i;

			Stat st;
			st = fs.stat(file_i.c_str());
			hash_map::set(_paths, resource_name, st);

			notify_add_file(resource_name.c_str(), st);
		}
	}
}

void SourceIndex::scan(const HashMap<DynamicString, DynamicString> &source_dirs)
{
	auto cur = hash_map::begin(source_dirs);
	auto end = hash_map::end(source_dirs);
	for (; cur != end; ++cur) {
		HASH_MAP_SKIP_HOLE(source_dirs, cur);

		TempAllocator512 ta;
		DynamicString prefix(ta);
		path::join(prefix, cur->second.c_str(), cur->first.c_str());

		FilesystemDisk fs(default_allocator());
		fs.set_prefix(prefix.c_str());
		scan_directory(fs, cur->first.c_str(), NULL);
	}
}

struct LineReader
{
	const char *_str;
	const u32 _len;
	u32 _pos;

	explicit LineReader(const char *str)
		: _str(str)
		, _len(strlen32(str))
		, _pos(0)
	{
	}

	void read_line(DynamicString &line)
	{
		const char *s  = &_str[_pos];
		const char *nl = strnl(s);
		_pos += u32(nl - s);
		line.set(s, u32(nl - s));
	}

	bool eof() const
	{
		return _str[_pos] == '\0';
	}
};

static void console_command_compile(ConsoleServer &cs, u32 client_id, const char *json, void *user_data)
{
	TempAllocator4096 ta;
	JsonObject obj(ta);
	DynamicString id(ta);
	DynamicString data_dir(ta);
	DynamicString platform(ta);
	StringStream ss(ta);
	DataCompiler *dc = (DataCompiler *)user_data;

	sjson::parse(obj, json);
	sjson::parse_string(id, obj["id"]);
	sjson::parse_string(data_dir, obj["data_dir"]);
	sjson::parse_string(platform, obj["platform"]);

	ss << "{";
	ss << "\"type\":\"compile\",";
	ss << "\"id\":\"" << id.c_str() << "\",";
	ss << "\"start\":true";
	ss << "}";
	cs.send(client_id, string_stream::c_str(ss));

	bool succ = dc->compile(data_dir.c_str(), platform.c_str());

	array::clear(ss);
	ss << "{";
	ss << "\"type\":\"compile\",";
	ss << "\"id\":\"" << id.c_str() << "\",";
	ss << "\"success\":" << (succ ? "true" : "false") << ",";
	ss << "\"revision\":" << dc->_revision;
	ss << "}";

	cs.send(client_id, string_stream::c_str(ss));
}

static void console_command_quit(ConsoleServer & /*cs*/, u32 /*client_id*/, const char * /*json*/, void * /*user_data*/)
{
	_quit = true;
}

static void console_command_refresh_list(ConsoleServer &cs, u32 client_id, const char *json, void *user_data)
{
	DataCompiler *dc = (DataCompiler *)user_data;

	TempAllocator4096 ta;
	StringStream ss(ta);
	JsonObject obj(ta);
	sjson::parse(obj, json);

	const u32 revision = (u32)sjson::parse_int(obj["revision"]);

	ss << "{\"type\":\"refresh_list\",\"list\":[";
	auto cur = hash_map::begin(dc->_data_revisions);
	auto end = hash_map::end(dc->_data_revisions);
	for (; cur != end; ++cur) {
		HASH_MAP_SKIP_HOLE(dc->_data_revisions, cur);

		DynamicString deffault(ta);
		if (cur->second > revision) {
			ss << "\"";
			ss << hash_map::get(dc->_data_index, cur->first, deffault).c_str();
			ss << "\",";
		}
	}
	ss << "]}";

	cs.send(client_id, string_stream::c_str(ss));
}

static Buffer read(FilesystemDisk &data_fs, const char *filename)
{
	Buffer buffer(default_allocator());

	File *file = data_fs.open(filename, FileOpenMode::READ);
	if (file->is_open()) {
		u32 size = file->size();
		if (size == 0) {
			data_fs.close(*file);
			return buffer;
		}

		array::resize(buffer, size);
		file->read(array::begin(buffer), size);
	}
	data_fs.close(*file);

	return buffer;
}

static void parse_data_versions(HashMap<StringId64, u32> &versions
	, Buffer &json
	)
{
	TempAllocator512 ta;
	JsonObject obj(ta);
	sjson::parse(obj, json);

	auto cur = json_object::begin(obj);
	auto end = json_object::end(obj);
	for (; cur != end; ++cur) {
		JSON_OBJECT_SKIP_HOLE(obj, cur);

		TempAllocator256 ta;
		DynamicString type_str(ta);
		type_str.set(cur->first.data(), cur->first.length());

		hash_map::set(versions, StringId64(type_str.c_str()), (u32)sjson::parse_int(cur->second));
	}
}

static void read_data_versions(HashMap<StringId64, u32> &versions
	, FilesystemDisk &data_fs
	, const char *filename
	)
{
	Buffer json = read(data_fs, filename);
	parse_data_versions(versions, json);
}

static void parse_data_index(HashMap<StringId64, DynamicString> &index
	, const SourceIndex &sources
	, Buffer &json
	)
{
	TempAllocator512 ta;
	JsonObject obj(ta);
	sjson::parse(obj, json);

	auto cur = json_object::begin(obj);
	auto end = json_object::end(obj);
	for (; cur != end; ++cur) {
		JSON_OBJECT_SKIP_HOLE(obj, cur);

		TempAllocator256 ta;
		DynamicString path(ta);
		sjson::parse_string(path, cur->second);

		// Skip reading data that belongs to non-existent source file.
		if (!hash_map::has(sources._paths, path))
			continue;

		StringId64 id;
		id.parse(cur->first.data());

		hash_map::set(index, id, path);
	}
}

static void read_data_index(HashMap<StringId64, DynamicString> &index
	, FilesystemDisk &data_fs
	, const SourceIndex &sources
	, const char *filename
	)
{
	Buffer json = read(data_fs, filename);
	parse_data_index(index, sources, json);
}

static void parse_data_mtimes(HashMap<StringId64, u64> &mtimes
	, const HashMap<StringId64, DynamicString> &data_index
	, Buffer &json
	)
{
	TempAllocator128 ta;
	JsonObject obj(ta);
	sjson::parse(obj, json);

	auto cur = json_object::begin(obj);
	auto end = json_object::end(obj);
	for (; cur != end; ++cur) {
		JSON_OBJECT_SKIP_HOLE(obj, cur);

		StringId64 id;
		id.parse(cur->first.data());

		// Skip reading data that belongs to non-existent source file.
		if (!hash_map::has(data_index, id))
			continue;

		TempAllocator64 ta;
		DynamicString mtime_json(ta);
		sjson::parse_string(mtime_json, cur->second);

		u64 mtime = strtoull(mtime_json.c_str(), NULL, 10);
		hash_map::set(mtimes, id, mtime);
	}
}

static void read_data_mtimes(HashMap<StringId64, u64> &mtimes
	, FilesystemDisk &data_fs
	, const HashMap<StringId64, DynamicString> &data_index
	, const char *filename
	)
{
	Buffer json = read(data_fs, filename);
	parse_data_mtimes(mtimes, data_index, json);
}

static void add_dependency_internal(HashMap<StringId64, HashMap<DynamicString, u32>> &dependencies, ResourceId id, const DynamicString &dependency)
{
	HashMap<DynamicString, u32> deps_deffault(default_allocator());
	HashMap<DynamicString, u32> &deps = hash_map::get(dependencies, id, deps_deffault);

	hash_map::set(deps, dependency, 0u);

	if (&deps == &deps_deffault)
		hash_map::set(dependencies, id, deps);
}

static void add_dependency_internal(HashMap<StringId64, HashMap<DynamicString, u32>> &dependencies, ResourceId id, const char *dependency)
{
	TempAllocator512 ta;
	DynamicString dependency_str(ta);
	dependency_str = dependency;

	add_dependency_internal(dependencies, id, dependency_str);
}

static void read_data_dependencies(DataCompiler &dc
	, FilesystemDisk &data_fs
	, const HashMap<StringId64, DynamicString> &data_index
	, const char *filename
	)
{
	Buffer json = read(data_fs, filename);

	TempAllocator1024 ta;
	JsonObject obj(ta);
	sjson::parse(obj, json);

	auto cur = json_object::begin(obj);
	auto end = json_object::end(obj);
	for (; cur != end; ++cur) {
		JSON_OBJECT_SKIP_HOLE(obj, cur);

		StringId64 id;
		id.parse(cur->first.data());

		// Skip reading data that belongs to non-existent source file.
		if (!hash_map::has(data_index, id))
			continue;

		JsonArray dependency_array(ta);
		sjson::parse_array(dependency_array, cur->second);
		for (u32 i = 0; i < array::size(dependency_array); ++i) {
			DynamicString path(ta);
			sjson::parse_string(path, dependency_array[i]);
			if (path.has_prefix("//r ")) {
				add_dependency_internal(dc._data_requirements, id, path.c_str() + 4);
			} else if (path.has_prefix("//- ")) {
				add_dependency_internal(dc._data_dependencies, id, path.c_str() + 4);
			} else { // Assume regular dependency
				add_dependency_internal(dc._data_dependencies, id, path.c_str());
			}
		}
	}
}

static s32 write_data_index(FilesystemDisk &data_fs, const char *filename, const HashMap<StringId64, DynamicString> &index)
{
	StringStream ss(default_allocator());
	DynamicString temp_filename(default_allocator());

	File *file = data_fs.open_temporary(temp_filename);
	if (file->is_open()) {
		auto cur = hash_map::begin(index);
		auto end = hash_map::end(index);
		for (; cur != end; ++cur) {
			HASH_MAP_SKIP_HOLE(index, cur);

			TempAllocator256 ta;
			DynamicString str(ta);
			str.from_string_id(cur->first);
			ss << "\"" << str.c_str() << "\" = \"" << cur->second.c_str() << "\"\n";
		}

		u32 ss_len = strlen32(string_stream::c_str(ss));
		if (ss_len != file->write(string_stream::c_str(ss), ss_len)) {
			data_fs.close(*file);
			return -1;
		}
	}
	data_fs.close(*file);

	RenameResult rr = data_fs.rename(temp_filename.c_str(), filename);
	return rr.error == RenameResult::SUCCESS ? 0 : -1;
}

static s32 write_data_versions(FilesystemDisk &data_fs
	, const char *filename
	, const HashMap<StringId64, u32> &versions
	, const HashMap<StringId64, DataCompiler::ResourceTypeData> &compilers
	)
{
	StringStream ss(default_allocator());
	DynamicString temp_filename(default_allocator());

	File *file = data_fs.open_temporary(temp_filename);
	if (file->is_open()) {
		auto cur = hash_map::begin(versions);
		auto end = hash_map::end(versions);
		for (; cur != end; ++cur) {
			HASH_MAP_SKIP_HOLE(versions, cur);

			DataCompiler::ResourceTypeData rtd;
			rtd.type_str = NULL;
			rtd = hash_map::get(compilers, cur->first, rtd);
			if (rtd.type_str != NULL)
				ss << rtd.type_str << " = " << cur->second << "\n";
		}

		u32 ss_len = strlen32(string_stream::c_str(ss));
		if (ss_len != file->write(string_stream::c_str(ss), ss_len)) {
			data_fs.close(*file);
			return -1;
		}
	}
	data_fs.close(*file);

	RenameResult rr = data_fs.rename(temp_filename.c_str(), filename);
	return rr.error == RenameResult::SUCCESS ? 0 : -1;
}

static s32 write_data_mtimes(FilesystemDisk &data_fs, const char *filename, const HashMap<StringId64, u64> &mtimes)
{
	StringStream ss(default_allocator());
	DynamicString temp_filename(default_allocator());

	File *file = data_fs.open_temporary(temp_filename);
	if (file->is_open()) {
		auto cur = hash_map::begin(mtimes);
		auto end = hash_map::end(mtimes);
		for (; cur != end; ++cur) {
			HASH_MAP_SKIP_HOLE(mtimes, cur);

			TempAllocator64 ta;
			DynamicString key(ta);
			key.from_string_id(cur->first);
			ss << "\"" << key.c_str() << "\" = \"" << cur->second << "\"\n";
		}

		u32 ss_len = strlen32(string_stream::c_str(ss));
		if (ss_len != file->write(string_stream::c_str(ss), ss_len)) {
			data_fs.close(*file);
			return -1;
		}
	}
	data_fs.close(*file);

	RenameResult rr = data_fs.rename(temp_filename.c_str(), filename);
	return rr.error == RenameResult::SUCCESS ? 0 : -1;
}

static s32 write_data_dependencies(FilesystemDisk &data_fs, const char *filename, const HashMap<StringId64, DynamicString> &index, const HashMap<StringId64, HashMap<DynamicString, u32>> &dependencies, const HashMap<StringId64, HashMap<DynamicString, u32>> &requirements)
{
	StringStream ss(default_allocator());
	DynamicString temp_filename(default_allocator());

	File *file = data_fs.open_temporary(temp_filename);
	if (file->is_open()) {
		auto cur = hash_map::begin(index);
		auto end = hash_map::end(index);
		for (; cur != end; ++cur) {
			HASH_MAP_SKIP_HOLE(index, cur);

			HashMap<DynamicString, u32> deps_deffault(default_allocator());
			const HashMap<DynamicString, u32> &deps = hash_map::get(dependencies, cur->first, deps_deffault);
			HashMap<DynamicString, u32> reqs_deffault(default_allocator());
			const HashMap<DynamicString, u32> &reqs = hash_map::get(requirements, cur->first, reqs_deffault);

			// Skip if data has no dependencies
			if (hash_map::size(deps) == 0 && hash_map::size(reqs) == 0)
				continue;

			TempAllocator64 ta;
			DynamicString key(ta);
			key.from_string_id(cur->first);
			ss << "\"" << key.c_str() << "\" = [\n";

			// Write all dependencies
			auto deps_cur = hash_map::begin(deps);
			auto deps_end = hash_map::end(deps);
			for (; deps_cur != deps_end; ++deps_cur) {
				HASH_MAP_SKIP_HOLE(deps, deps_cur);

				ss << "    \"//- " << deps_cur->first.c_str() << "\"\n";
			}

			// Write all requirements
			auto reqs_cur = hash_map::begin(reqs);
			auto reqs_end = hash_map::end(reqs);
			for (; reqs_cur != reqs_end; ++reqs_cur) {
				HASH_MAP_SKIP_HOLE(reqs, reqs_cur);

				ss << "    \"//r " << reqs_cur->first.c_str() << "\"\n";
			}

			ss << "]\n";
		}

		u32 ss_len = strlen32(string_stream::c_str(ss));
		if (ss_len != file->write(string_stream::c_str(ss), ss_len)) {
			data_fs.close(*file);
			return -1;
		}
	}
	data_fs.close(*file);

	RenameResult rr = data_fs.rename(temp_filename.c_str(), filename);
	return rr.error == RenameResult::SUCCESS ? 0 : -1;
}

DataCompiler::DataCompiler(const DeviceOptions &opts, ConsoleServer &cs)
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
	, _data_revisions(default_allocator())
	, _revision(0)
{
	cs.register_message_type("compile", console_command_compile, this);
	cs.register_message_type("quit", console_command_quit, this);
	cs.register_message_type("refresh_list", console_command_refresh_list, this);
}

DataCompiler::~DataCompiler()
{
	if (_options->_server)
		_file_monitor.stop();
}

void DataCompiler::add_file(const char *path)
{
	// Get source directory prefix
	TempAllocator512 ta;
	DynamicString source_dir(ta);
	source_dir = hash_map::get(_source_dirs, source_dir, source_dir);

	// Convert to DynamicString
	DynamicString str(ta);
	str.set(path, strlen32(path));

	Stat deff_st;
	deff_st.file_type = Stat::NO_ENTRY;
	deff_st.size = 0;
	deff_st.mtime = 0;
	Stat prev_st = hash_map::get(_source_index._paths, str, deff_st);

	// Get file status.
	FilesystemDisk fs(default_allocator());
	fs.set_prefix(source_dir.c_str());
	Stat st;
	st = fs.stat(path);
	hash_map::set(_source_index._paths, str, st);

	// Avoid sending spurious add_file() notifications for already known paths.
	if (prev_st.file_type == Stat::NO_ENTRY)
		notify_add_file(path, st);
}

void DataCompiler::remove_file(const char *path)
{
	TempAllocator512 ta;
	DynamicString path_str(ta);
	path_str.set(path, strlen32(path));

	// Mark the entry as deleted but do not remove it from the map. We still
	// need to know which resource has been deleted in order to remove its
	// associated BLOB in the data directory at the next compile() call.
	Stat st;
	st.file_type = Stat::NO_ENTRY;
	st.size = 0;
	st.mtime = 0;
	hash_map::set(_source_index._paths, path_str, st);

	notify_remove_file(path);
}

void DataCompiler::add_tree(const char *path)
{
	notify_add_tree(path);
}

void DataCompiler::remove_tree(const char *path)
{
	TempAllocator512 ta;
	DynamicString tree_path(ta);
	tree_path = path;
	tree_path += '/';

	Vector<DynamicString> dangling_paths(default_allocator());

	auto cur = hash_map::begin(_source_index._paths);
	auto end = hash_map::end(_source_index._paths);
	for (; cur != end; ++cur) {
		HASH_MAP_SKIP_HOLE(_source_index._paths, cur);

		// Skip if it is not a sub-path
		if (!cur->first.has_prefix(tree_path.c_str()))
			continue;

		// Skip if is has been deleted previously
		if (cur->second.file_type == Stat::NO_ENTRY)
			continue;

		vector::push_back(dangling_paths, cur->first);
	}

	for (u32 ii = 0; ii < vector::size(dangling_paths); ++ii)
		remove_file(dangling_paths[ii].c_str());

	notify_remove_tree(path);
}

void DataCompiler::remove_file_or_tree(const char *path)
{
	TempAllocator512 ta;
	DynamicString path_str(ta);
	path_str = path;

	if (hash_map::has(_source_index._paths, path_str))
		remove_file(path);
	else
		remove_tree(path);
}

s32 DataCompiler::map_source_dir(const char *name, const char *source_dir)
{
	TempAllocator256 ta;
	DynamicString sname(ta);
	DynamicString sdir(ta);

	Stat st;
	os::stat(st, source_dir);
	if (st.file_type != Stat::DIRECTORY)
		return -1;

	sname.set(name, strlen32(name));
	sdir.set(source_dir, strlen32(source_dir));
	hash_map::set(_source_dirs, sname, sdir);
	return 0;
}

void DataCompiler::source_dir(const char *resource_name, DynamicString &source_dir)
{
	const char *slash = strchr(resource_name, '/');

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

void DataCompiler::add_ignore_glob(const char *glob)
{
	TempAllocator64 ta;
	DynamicString str(ta);
	str.set(glob, strlen32(glob));
	vector::push_back(_globs, str);
}

void DataCompiler::scan_and_restore(const char *data_dir)
{
	// Scan all source directories
	s64 time_start = time::now();

	// FIXME: refactor this whole garbage
	Array<const char *> directories(default_allocator());

	auto cur = hash_map::begin(_source_dirs);
	auto end = hash_map::end(_source_dirs);
	for (; cur != end; ++cur) {
		HASH_MAP_SKIP_HOLE(_source_dirs, cur);

		DynamicString prefix(default_allocator());
		path::join(prefix, cur->second.c_str(), cur->first.c_str());

		char *str = (char *)default_allocator().allocate(prefix.length() + 1);
		strcpy(str, prefix.c_str());
		array::push_back(directories, (const char *)str);

		_source_fs.set_prefix(prefix.c_str());

		File *file = _source_fs.open(CROWN_DATAIGNORE, FileOpenMode::READ);
		if (file->is_open()) {
			const u32 size = file->size();
			char *data = (char *)default_allocator().allocate(size + 1);
			file->read(data, size);
			data[size] = '\0';

			LineReader lr(data);

			while (!lr.eof()) {
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
		_source_fs.close(*file);
	}

	_source_index.scan(_source_dirs);

	logi(DATA_COMPILER, "Scanned data in " TIME_FMT, time::seconds(time::now() - time_start));

	// Restore state from previous run
	time_start = time::now();

	FilesystemDisk data_fs(default_allocator());
	data_fs.set_prefix(data_dir);

	read_data_index(_data_index, data_fs, _source_index, CROWN_DATA_INDEX);
	read_data_mtimes(_data_mtimes, data_fs, _data_index, CROWN_DATA_MTIMES);
	read_data_dependencies(*this, data_fs, _data_index, CROWN_DATA_DEPENDENCIES);
	read_data_versions(_data_versions, data_fs, CROWN_DATA_VERSIONS);
	logi(DATA_COMPILER, "Restored state in " TIME_FMT, time::seconds(time::now() - time_start));

	if (_options->_server) {
		// Start file monitor
		time_start = time::now();
		_file_monitor.start(array::size(directories)
			, array::begin(directories)
			, true
			, file_monitor_callback
			, this
			);
		logi(DATA_COMPILER, "Started file monitor in " TIME_FMT, time::seconds(time::now() - time_start));
	}

	// Cleanup
	for (u32 i = 0, n = array::size(directories); i < n; ++i)
		default_allocator().deallocate((void *)directories[n - 1 - i]);
}

bool DataCompiler::dependency_changed(const DynamicString &path, ResourceId id, u64 dst_mtime)
{
	Stat st;
	st.file_type = Stat::FileType::NO_ENTRY;
	st.size = 0;
	st.mtime = 0;
	st = hash_map::get(_source_index._paths, path, st);
	if (st.file_type == Stat::FileType::NO_ENTRY || st.mtime > dst_mtime)
		return true;

	const HashMap<DynamicString, u32> deffault(default_allocator());
	const HashMap<DynamicString, u32> &deps = hash_map::get(_data_dependencies, id, deffault);
	auto cur = hash_map::begin(deps);
	auto end = hash_map::end(deps);
	for (; cur != end; ++cur) {
		HASH_MAP_SKIP_HOLE(deps, cur);

		if (path == cur->first)
			continue;

		if (dependency_changed(cur->first, resource_id(cur->first.c_str()), dst_mtime))
			return true;
	}

	return false;
}

bool DataCompiler::version_changed(const DynamicString &path, ResourceId id)
{
	const StringId64 type(resource_type(path.c_str()));
	const u32 dv = data_version(type);
	const u32 ds = data_version_stored(type);

	if (dv == UINT32_MAX)
		return false;
	else if (dv != ds)
		return true;

	const HashMap<DynamicString, u32> deffault(default_allocator());
	const HashMap<DynamicString, u32> &deps = hash_map::get(_data_dependencies, id, deffault);
	auto cur = hash_map::begin(deps);
	auto end = hash_map::end(deps);
	for (; cur != end; ++cur) {
		HASH_MAP_SKIP_HOLE(deps, cur);

		if (path == cur->first)
			continue;

		if (version_changed(cur->first, resource_id(cur->first.c_str())))
			return true;
	}

	return false;
}

bool DataCompiler::path_matches_ignore_glob(const char *path)
{
	for (u32 ii = 0, nn = vector::size(_globs); ii < nn; ++ii) {
		if (wildcmp(_globs[ii].c_str(), path))
			return true;
	}

	return false;
}

bool DataCompiler::path_is_special(const char *path)
{
	return strcmp(path, "_level_editor_test.level") == 0
		|| strcmp(path, "_level_editor_test.package") == 0
		;
}

bool DataCompiler::compile_internal(const char *data_dir, const char *platform_name)
{
	s64 time_start = time::now();

	Platform::Enum platform = Platform::COUNT;
	for (u32 ii = 0; ii < countof(platform_info); ++ii) {
		if (strcmp(platform_info[ii].name, platform_name) == 0)
			platform = platform_info[ii].type;
	}

	if (platform == Platform::COUNT) {
		loge(DATA_COMPILER, "Cannot compile data for unknown platform `%s`", platform_name);
		return false;
	}

	FilesystemDisk data_fs(default_allocator());
	data_fs.set_prefix(data_dir);

	// Create the data directory on disk.
	CreateResult cr;
	cr = data_fs.create_directory("");
	if (cr.error == CreateResult::SUCCESS || cr.error == CreateResult::ALREADY_EXISTS) {
		if (cr.error == CreateResult::SUCCESS) {
			// Either the data directory has never been created before or it has
			// been deleted while the data compiler was running. In both cases reset
			// the tracking structures to force a full compile.
			hash_map::clear(_data_index);
			hash_map::clear(_data_mtimes);
			hash_map::clear(_data_dependencies);
			hash_map::clear(_data_requirements);
			hash_map::clear(_data_versions);
		}

		// Create sub-directories.
		data_fs.create_directory(CROWN_DATA_DIRECTORY);
		data_fs.create_directory(CROWN_TEMP_DIRECTORY);
	} else {
		loge(DATA_COMPILER, "Failed to create the data directory: `%s`", data_dir);
		return false;
	}

	// Find the set of resources to be compiled, removed etc.
	Vector<DynamicString> to_compile(default_allocator());
	Vector<DynamicString> to_remove(default_allocator());

	auto cur = hash_map::begin(_source_index._paths);
	auto end = hash_map::end(_source_index._paths);
	for (; cur != end; ++cur) {
		HASH_MAP_SKIP_HOLE(_source_index._paths, cur);

		const DynamicString &path = cur->first;

		if (cur->second.file_type == Stat::NO_ENTRY) {
			vector::push_back(to_remove, path);
		} else {
			if (path_matches_ignore_glob(path.c_str()))
				continue;

			const char *type_str = resource_type(path.c_str());
			if (type_str == NULL)
				continue;

			const ResourceId id = resource_id(path.c_str());

			const u64 mtime_epoch = 0u;
			const u64 mtime = hash_map::get(_data_mtimes, id, mtime_epoch);

			bool source_never_compiled_before    = hash_map::has(_data_index, id) == false;
			bool source_dependency_changed       = dependency_changed(path, id, mtime);
			bool data_version_dependency_changed = version_changed(path, id);
			bool compile_always                  = data_version(StringId64(type_str)) == 0u;

			if (source_never_compiled_before
				|| source_dependency_changed
				|| data_version_dependency_changed
				|| compile_always
				) {
				vector::push_back(to_compile, path);
			}
		}
	}

#if 0
	for (u32 i = 0; i < vector::size(to_remove); ++i)
		logi(DATA_COMPILER, "gc %s", to_remove[i].c_str());
#endif

	// Remove all deleted resources
	for (u32 i = 0; i < vector::size(to_remove); ++i) {
		// Remove from source index
		hash_map::remove(_source_index._paths, to_remove[i]);

		// If it does not have extension it cannot be a resource so it cannot be
		// in tracking structures nor in the data folder.
		if (resource_type(to_remove[i].c_str()) == NULL)
			continue;

		// Remove from tracking structures
		ResourceId id = resource_id(to_remove[i].c_str());
		hash_map::remove(_data_index, id);
		hash_map::remove(_data_mtimes, id);
		hash_map::remove(_data_dependencies, id);
		hash_map::remove(_data_requirements, id);
		hash_map::remove(_data_revisions, id);

		// If present, remove from data folder because we do not want the
		// runtime to accidentally load stale data compiled from resources that
		// do not exist anymore in the source index.
		TempAllocator256 ta;
		DynamicString dest(ta);
		destination_path(dest, id);
		data_fs.delete_file(dest.c_str());
	}

	// Sort to_compile so that ".package" resources get compiled last
	std::sort(vector::begin(to_compile)
		, vector::end(to_compile)
		, [](const DynamicString &resource_a, const DynamicString &resource_b) {
#define PACKAGE ".package"
			if (resource_a.has_suffix(PACKAGE) && !resource_b.has_suffix(PACKAGE))
				return false;
			if (!resource_a.has_suffix(PACKAGE) && resource_b.has_suffix(PACKAGE))
				return true;
			return resource_a < resource_b;
#undef PACKAGE
		});

	bool success = true;

	// Compile all changed resources
	for (u32 i = 0; i < vector::size(to_compile); ++i) {
		const DynamicString &path = to_compile[i];

		const char *type_str = resource_type(path.c_str());
		if (type_str == NULL)
			continue;

		const StringId64 type(type_str);
		if (!can_compile(type))
			continue;

		logi(DATA_COMPILER, _options->_server ? RESOURCE_ID_FMT_STR : "%s", path.c_str());

		// Build destination file path
		const ResourceId id = resource_id(path.c_str());
		TempAllocator256 ta;
		DynamicString dest(ta);
		destination_path(dest, id);

		// Dependencies and requirements lists must be regenerated each time
		// the resource is being compiled. For example, if you delete
		// "foo.unit" from a package, you do not want the list of
		// requirements to include "foo.unit" again the next time that
		// package is compiled.
		HashMap<DynamicString, u32> new_dependencies(default_allocator());
		HashMap<DynamicString, u32> new_requirements(default_allocator());

		// Compile data.
		ResourceTypeData rtd;
		rtd.version = 0;
		rtd.compiler = NULL;
		rtd = hash_map::get(_compilers, type, rtd);

		Buffer output_buffer(default_allocator());
		FileBuffer output(output_buffer);
		Buffer stream_output_buffer(default_allocator());
		FileBuffer stream_output(stream_output_buffer);
		CompileOptions opts(output
			, stream_output
			, new_dependencies
			, new_requirements
			, *this
			, data_fs
			, data_fs
			, id
			, path
			, platform
			, false
			);

		success = rtd.compiler(opts) == 0;

		if (success) {
			// Update dependencies and requirements only if compiler(opts)
			// succeeded. If the compilation fails due to a missing
			// dependency and you update the dependency database with new
			// partial data, the next call to compile() would not trigger a
			// recompilation.
			HashMap<DynamicString, u32> dependencies_deffault(default_allocator());
			hash_map::clear(hash_map::get(_data_dependencies, id, dependencies_deffault));
			HashMap<DynamicString, u32> requirements_deffault(default_allocator());
			hash_map::clear(hash_map::get(_data_requirements, id, requirements_deffault));
			hash_map::set(_data_dependencies, id, new_dependencies);
			{
				// Add requirements from globs.
				auto cur = hash_map::begin(_source_index._paths);
				auto end = hash_map::end(_source_index._paths);
				for (; cur != end; ++cur) {
					HASH_MAP_SKIP_HOLE(_source_index._paths, cur);

					const DynamicString &path = cur->first;
					for (u32 ii = 0, nn = vector::size(opts._new_requirement_globs); ii < nn; ++ii) {
						if (wildcmp(opts._new_requirement_globs[ii].c_str(), path.c_str()))
							hash_map::set(new_requirements, path, 0u);
					}
				}
			}
			hash_map::set(_data_requirements, id, new_requirements);

			// Write output data to disk.
			DynamicString temp_dest(default_allocator());
			File *outf = data_fs.open_temporary(temp_dest);
			if (outf->is_open()) {
				u32 size = array::size(output_buffer);
				u32 written = outf->write(array::begin(output_buffer), size);
				success = size == written;
			} else {
				loge(DATA_COMPILER, "Failed to write data to disk");
				success = false;
			}
			data_fs.close(*outf);

			if (success) {
				RenameResult rr = data_fs.rename(temp_dest.c_str(), dest.c_str());
				success = rr.error == RenameResult::SUCCESS;
			}

			if (success) {
				// Write streaming output data to disk, if any.
				if (array::size(stream_output_buffer) != 0) {
					DynamicString temp_dest(default_allocator());
					File *outf = data_fs.open_temporary(temp_dest);
					if (outf->is_open()) {
						u32 size = array::size(stream_output_buffer);
						u32 written = outf->write(array::begin(stream_output_buffer), size);
						success = size == written;
					} else {
						loge(DATA_COMPILER, "Failed to write streaming data to disk");
						success = false;
					}
					data_fs.close(*outf);

					if (success) {
						TempAllocator256 ta;
						DynamicString stream_dest(ta);
						stream_destination_path(stream_dest, id);
						RenameResult rr = data_fs.rename(temp_dest.c_str(), stream_dest.c_str());
						success = rr.error == RenameResult::SUCCESS;
					}
				}
			}
		}

		if (success) {
			// Do not include special paths in content tracking structures.
			if (!path_is_special(path.c_str())) {
				hash_map::set(_data_index, id, path);
				hash_map::set(_data_mtimes, id, data_fs.last_modified_time(dest.c_str()));
				hash_map::set(_data_revisions, id, _revision + 1);
			}
		} else {
			loge(DATA_COMPILER, "Failed to compile data");
			break;
		}
	}

	if (success) {
		// Data versions are stored per-type, so, before updating _data_versions, we
		// need to make sure *all* resource files with that type have been
		// successfully compiled.
		auto cur = hash_map::begin(_compilers);
		auto end = hash_map::end(_compilers);
		for (; cur != end; ++cur) {
			HASH_MAP_SKIP_HOLE(_compilers, cur);

			hash_map::set(_data_versions, cur->first, cur->second.version);
		}

		if (vector::size(to_compile)) {
			_revision++;
			logi(DATA_COMPILER, "Data (rev %u) compiled in " TIME_FMT, _revision, time::seconds(time::now() - time_start));
		} else {
			logi(DATA_COMPILER, "Data is up to date");
		}

		if (_options->_do_bundle) {
			time_start = time::now();
			// Find the set of resources to be compiled, removed etc.
			Vector<DynamicString> to_bundle(default_allocator());

			// Find all packages.
			auto cur = hash_map::begin(_source_index._paths);
			auto end = hash_map::end(_source_index._paths);
			for (; cur != end; ++cur) {
				HASH_MAP_SKIP_HOLE(_source_index._paths, cur);

				const DynamicString &path = cur->first;

				if (path.has_suffix(".package") || path.has_suffix(".config"))
					vector::push_back(to_bundle, path);
			}

			FilesystemDisk data_fs(default_allocator());
			data_fs.set_prefix(data_dir);

			const char *bundle_dir = _options->_bundle_dir.c_str();
			// Create the bundle directory on disk.
			FilesystemDisk bundle_fs(default_allocator());
			bundle_fs.set_prefix(bundle_dir);

			cr = bundle_fs.create_directory("");
			if (cr.error == CreateResult::SUCCESS || cr.error == CreateResult::ALREADY_EXISTS) {
				bundle_fs.create_directory(CROWN_DATA_DIRECTORY);
			} else {
				loge(DATA_COMPILER, "Failed to create the bundle directory: `%s`", bundle_dir);
				return false;
			}

			for (u32 ii = 0; ii < vector::size(to_bundle); ++ii) {
				const DynamicString &path = to_bundle[ii];
				logi(DATA_COMPILER, _options->_server ? RESOURCE_ID_FMT_STR : "%s", path.c_str());

				ResourceId id = resource_id(path.c_str());
				TempAllocator256 ta;
				DynamicString dest(ta);
				destination_path(dest, id);

				const StringId64 type(resource_type(path.c_str()));

				HashMap<DynamicString, u32> new_dependencies(default_allocator());
				HashMap<DynamicString, u32> new_requirements(default_allocator());
				Buffer ouput_buffer(default_allocator());
				FileBuffer output(ouput_buffer);
				Buffer stream_ouput_buffer(default_allocator());
				FileBuffer stream_output(stream_ouput_buffer);
				CompileOptions opts(output
					, stream_output
					, new_dependencies
					, new_requirements
					, *this
					, bundle_fs
					, data_fs
					, id
					, path
					, platform
					, true
					);

				// Bundle data.
				ResourceTypeData rtd;
				rtd.version = 0;
				rtd.compiler = NULL;
				rtd = hash_map::get(_compilers, type, rtd);
				success = rtd.compiler(opts) == 0;

				if (success) {
					// Write data to disk.
					DynamicString temp_dest(default_allocator());
					File *outf = bundle_fs.open_temporary(temp_dest);
					if (outf->is_open()) {
						u32 size = array::size(ouput_buffer);
						u32 written = outf->write(array::begin(ouput_buffer), size);
						success = size == written;
					} else {
						loge(DATA_COMPILER, "Failed to write data to disk");
						success = false;
					}
					bundle_fs.close(*outf);

					if (success) {
						RenameResult rr = bundle_fs.rename(temp_dest.c_str(), dest.c_str());
						success = rr.error == RenameResult::SUCCESS;
					}
				}

				if (!success) {
					loge(DATA_COMPILER, "Failed to generate bundle");
					break;
				}
			}

			if (success) {
				if (vector::size(to_bundle)) {
					logi(DATA_COMPILER, "Bundled data in " TIME_FMT, time::seconds(time::now() - time_start));
				} else {
					logi(DATA_COMPILER, "Bundles are up to date");
				}
			}
		}
	}

	// Save state to disk.
	s32 res = write_data_index(data_fs, CROWN_DATA_INDEX, _data_index);
	if (res != 0) {
		loge(DATA_COMPILER, "Failed to save: %s", CROWN_DATA_INDEX);
		return false;
	}

	res = write_data_mtimes(data_fs, CROWN_DATA_MTIMES, _data_mtimes);
	if (res != 0) {
		loge(DATA_COMPILER, "Failed to save: %s", CROWN_DATA_MTIMES);
		return false;
	}

	res = write_data_dependencies(data_fs, CROWN_DATA_DEPENDENCIES, _data_index, _data_dependencies, _data_requirements);
	if (res != 0) {
		loge(DATA_COMPILER, "Failed to save: %s", CROWN_DATA_DEPENDENCIES);
		return false;
	}

	res = write_data_versions(data_fs, CROWN_DATA_VERSIONS, _data_versions, _compilers);
	if (res != 0) {
		loge(DATA_COMPILER, "Failed to save: %s", CROWN_DATA_VERSIONS);
		return false;
	}

	return success;
}

bool DataCompiler::compile(const char *data_dir, const char *platform_name)
{
	profiler_globals::clear();
	bool success = compile_internal(data_dir, platform_name);
	profiler_globals::flush();
	return success;
}

void DataCompiler::register_compiler(const char *type_str, u32 version, CompileFunction compiler, void *user_data)
{
	StringId64 type(type_str);

	CE_ASSERT(!hash_map::has(_compilers, type), "Type already registered");
	CE_ENSURE(NULL != compiler);

	ResourceTypeData rtd;
	rtd.version = version;
	rtd.compiler = compiler;
	rtd.type_str = type_str;
	rtd.user_data = user_data;

	hash_map::set(_compilers, type, rtd);
}

u32 DataCompiler::data_version(StringId64 type)
{
	ResourceTypeData rtd;
	rtd.version = COMPILER_NOT_FOUND;
	rtd.compiler = NULL;
	return hash_map::get(_compilers, type, rtd).version;
}

u32 DataCompiler::data_version_stored(StringId64 type)
{
	u32 version = UINT32_MAX;
	return hash_map::get(_data_versions, type, version);
}

void *DataCompiler::user_data(StringId64 type)
{
	ResourceTypeData rtd;
	rtd.version = COMPILER_NOT_FOUND;
	rtd.compiler = NULL;
	return hash_map::get(_compilers, type, rtd).user_data;
}

bool DataCompiler::can_compile(StringId64 type)
{
	return hash_map::has(_compilers, type);
}

void DataCompiler::error(const char *msg, va_list args)
{
	vloge(DATA_COMPILER, msg, args);
}

/// Converts @a path to the corresponding resource name.
/// On Linux, no transformation is needed. On Windows,
/// backslashes are converted to slashes.
static void resource_path_to_resource_name(DynamicString &resource_name, const DynamicString &path)
{
	for (u32 i = 0, n = path.length(); i < n; ++i) {
		if (path._data[i] == '\\')
			resource_name += '/';
		else
			resource_name += path._data[i];
	}
}

void DataCompiler::file_monitor_callback(FileMonitorEvent::Enum fme, bool is_dir, const char *path, const char *path_renamed)
{
	TempAllocator512 ta;
	DynamicString source_dir(ta);
	DynamicString resource_path(ta); // Same as resource_name but with OS-dependent directory separators
	DynamicString resource_name(ta);

	// Find source directory by matching mapped
	// directory prefix with `path`.
	auto cur = hash_map::begin(_source_dirs);
	auto end = hash_map::end(_source_dirs);
	for (; cur != end; ++cur) {
		HASH_MAP_SKIP_HOLE(_source_dirs, cur);

		path::join(source_dir, cur->second.c_str(), cur->first.c_str());
		if (str_has_prefix(path, source_dir.c_str()))
			break;
	}

	if (cur != end) {
		// All events received must refer to directories
		// mapped with map_source_dir().
		const char *filename = &path[source_dir.length() + 1];
		path::join(resource_path, cur->first.c_str(), filename);
		resource_path_to_resource_name(resource_name, resource_path);

#if 0
		static const char *fme_to_name[] = { "CREATED", "DELETED", "RENAMED", "CHANGED" };
		CE_STATIC_ASSERT(countof(fme_to_name) == FileMonitorEvent::COUNT);
		logi(DATA_COMPILER, "file_monitor_callback: event: %s %s", fme_to_name[fme], is_dir ? "dir" : "file");
		logi(DATA_COMPILER, "  path         : %s", path);
		if (fme == FileMonitorEvent::RENAMED)
			logi(DATA_COMPILER, "  path_renamed : %s", path_renamed);
		logi(DATA_COMPILER, "  source_dir   : %s", source_dir.c_str());
		logi(DATA_COMPILER, "  resource_path: %s", resource_path.c_str());
		logi(DATA_COMPILER, "  resource_name: %s", resource_name.c_str());
#endif

		switch (fme) {
		case FileMonitorEvent::CREATED:
			if (!is_dir)
				add_file(resource_name.c_str());
			else
				add_tree(resource_name.c_str());
			break;

		case FileMonitorEvent::DELETED:
			remove_file_or_tree(resource_name.c_str());
			break;

		case FileMonitorEvent::RENAMED: {
			DynamicString resource_path_renamed(ta); // See resource_path
			DynamicString resource_name_renamed(ta);
			path::join(resource_path_renamed, cur->first.c_str(), &path_renamed[source_dir.length() + 1]);
			resource_path_to_resource_name(resource_name_renamed, resource_path_renamed);

			if (!is_dir) {
				remove_file(resource_name.c_str());
				add_file(resource_name_renamed.c_str());
			} else {
				remove_tree(resource_name.c_str());
				add_tree(resource_name_renamed.c_str());
			}
			break;
		}

		case FileMonitorEvent::CHANGED:
			if (!is_dir) {
				FilesystemDisk fs(default_allocator());
				fs.set_prefix(source_dir.c_str());

				Stat st;
				st = fs.stat(filename);
				hash_map::set(_source_index._paths, resource_name, st);

				notify_change_file(resource_name.c_str(), st);
			}
			break;

		default:
			CE_FATAL("Unknown FileMonitorEvent: %d", fme);
			break;
		}
	}
}

void DataCompiler::file_monitor_callback(void *thiz, FileMonitorEvent::Enum fme, bool is_dir, const char *path_original, const char *path_modified)
{
	((DataCompiler *)thiz)->file_monitor_callback(fme, is_dir, path_original, path_modified);
}

#define RESOURCE_TYPE(type_name)              \
	namespace type_name##_resource_internal   \
	{                                         \
		extern s32 compile(CompileOptions &); \
	}                                         \

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
int main_data_compiler(const DeviceOptions &opts)
{
#if CROWN_PLATFORM_WINDOWS
	// code-format off
	PHANDLER_ROUTINE signal_handler = [](DWORD dwCtrlType) {
			switch (dwCtrlType) {
			case CTRL_C_EVENT:
				_quit = true;
				if (console_server())
					console_server()->shutdown();
				return TRUE;

			default:
				return FALSE;
			}
		};
	SetConsoleCtrlHandler(signal_handler, TRUE);
#else
	struct sigaction old_SIGINT;
	struct sigaction old_SIGTERM;
	struct sigaction act;
	act.sa_handler = [](int signum) {
			switch (signum)
			{
			case SIGINT:
			case SIGTERM:
				_quit = true;
				if (console_server())
					console_server()->shutdown();
				break;

			default:
				break;
			}
		};
	sigemptyset(&act.sa_mask);
	act.sa_flags = 0;
	sigaction(SIGINT, NULL, &old_SIGINT);
	sigaction(SIGINT, &act, NULL);
	sigaction(SIGTERM, NULL, &old_SIGTERM);
	sigaction(SIGTERM, &act, NULL);
	// code-format on
#endif // if CROWN_PLATFORM_WINDOWS

	console_server_globals::init();
	if (opts._server)
		console_server()->listen(CROWN_DEFAULT_COMPILER_PORT, opts._wait_console);

	MeshCache mesh_cache;

	DataCompiler *dc = CE_NEW(default_allocator(), DataCompiler)(opts, *console_server());
	dc->register_compiler("config",           RESOURCE_VERSION_CONFIG,           config_resource_internal::compile);
	dc->register_compiler("font",             RESOURCE_VERSION_FONT,             font_resource_internal::compile);
	dc->register_compiler("level",            RESOURCE_VERSION_LEVEL,            level_resource_internal::compile);
	dc->register_compiler("material",         RESOURCE_VERSION_MATERIAL,         material_resource_internal::compile);
	dc->register_compiler("mesh",             RESOURCE_VERSION_MESH,             mesh_resource_internal::compile, &mesh_cache);
	dc->register_compiler("mesh_skeleton",    RESOURCE_VERSION_MESH_SKELETON,    mesh_skeleton_resource_internal::compile);
	dc->register_compiler("mesh_animation",   RESOURCE_VERSION_MESH_ANIMATION,   mesh_animation_resource_internal::compile);
	dc->register_compiler("package",          RESOURCE_VERSION_PACKAGE,          package_resource_internal::compile);
	dc->register_compiler("physics_config",   RESOURCE_VERSION_PHYSICS_CONFIG,   physics_config_resource_internal::compile);
	dc->register_compiler("render_config",    RESOURCE_VERSION_RENDER_CONFIG,    render_config_resource_internal::compile);
	dc->register_compiler("lua",              RESOURCE_VERSION_SCRIPT,           lua_resource_internal::compile);
	dc->register_compiler("shader",           RESOURCE_VERSION_SHADER,           shader_resource_internal::compile);
	dc->register_compiler("sound",            RESOURCE_VERSION_SOUND,            sound_resource_internal::compile);
	dc->register_compiler("sprite",           RESOURCE_VERSION_SPRITE,           sprite_resource_internal::compile);
	dc->register_compiler("sprite_animation", RESOURCE_VERSION_SPRITE_ANIMATION, sprite_animation_resource_internal::compile);
	dc->register_compiler("state_machine",    RESOURCE_VERSION_STATE_MACHINE,    state_machine_resource_internal::compile);
	dc->register_compiler("texture",          RESOURCE_VERSION_TEXTURE,          texture_resource_internal::compile);
	dc->register_compiler("unit",             RESOURCE_VERSION_UNIT,             unit_resource_internal::compile);

	s32 err = 0;

	if (dc->map_source_dir("", opts._source_dir.c_str()) != 0) {
		loge(DATA_COMPILER, "Cannot map source directory `%s`", opts._source_dir.c_str());
		err = EXIT_FAILURE;
		goto delete_dc;
	}

	if (opts._map_source_dir_name) {
		if (dc->map_source_dir(opts._map_source_dir_name
			, opts._map_source_dir_prefix.c_str()
			) != 0) {
			loge(DATA_COMPILER, "Cannot map source directory `%s`", opts._map_source_dir_prefix.c_str());
			err = EXIT_FAILURE;
			goto delete_dc;
		}
	}

	dc->scan_and_restore(opts._data_dir.c_str());

	if (opts._server) {
		while (!_quit) {
			console_server()->execute_message_handlers(true);
		}
	} else {
		err = dc->compile(opts._data_dir.c_str(), opts._platform) == true
			? EXIT_SUCCESS
			: EXIT_FAILURE
			;
	}

delete_dc:
	CE_DELETE(default_allocator(), dc);
	console_server_globals::shutdown();

	// Restore original signal handlers.
#if CROWN_PLATFORM_WINDOWS
	SetConsoleCtrlHandler(signal_handler, FALSE);
#else
	sigaction(SIGINT, &old_SIGINT, NULL);
	sigaction(SIGTERM, &old_SIGTERM, NULL);
#endif

	return err;
}

} // namespace crown

#endif // if CROWN_CAN_COMPILE
