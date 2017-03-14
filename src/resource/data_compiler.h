/*
 * Copyright (c) 2012-2017 Daniele Bartolini and individual contributors.
 * License: https://github.com/taylor001/crown/blob/master/LICENSE
 */

#pragma once

#include "compiler_types.h"
#include "console_server.h"
#include "container_types.h"
#include "file_monitor.h"
#include "filesystem_disk.h"

namespace crown
{
class DataCompiler
{
	typedef void (*CompileFunction)(const char* path, CompileOptions& opts);

	struct ResourceTypeData
	{
		u32 version;
		CompileFunction compiler;
	};

	ConsoleServer* _console_server;
	FilesystemDisk _source_fs;
	Map<DynamicString, DynamicString> _source_dirs;
	HashMap<StringId64, ResourceTypeData> _compilers;
	Vector<DynamicString> _files;
	Vector<DynamicString> _globs;
	FileMonitor _file_monitor;

	void add_file(const char* path);
	void add_tree(const char* path);
	void remove_file(const char* path);
	void remove_tree(const char* path);

	void scan_source_dir(const char* prefix, const char* path);

	bool can_compile(StringId64 type);
	void compile(StringId64 type, const char* path, CompileOptions& opts);
	bool compile(FilesystemDisk& bundle_fs, const char* type, const char* name, const char* platform);

	void filemonitor_callback(FileMonitorEvent::Enum fme, bool is_dir, const char* path, const char* path_renamed);
	static void filemonitor_callback(void* thiz, FileMonitorEvent::Enum fme, bool is_dir, const char* path_original, const char* path_modified);

public:

	DataCompiler(ConsoleServer& cs);
	~DataCompiler();

	void map_source_dir(const char* name, const char* source_dir);
	void source_dir(const char* resource_name, DynamicString& source_dir);

	void add_ignore_glob(const char* glob);

	/// Scans source tree for resources.
	void scan();

	/// Compiles all the resources found in the source tree and puts them in @a bundle_dir.
	/// Returns true on success, false otherwise.
	bool compile(const char* bundle_dir, const char* platform);

	/// Registers the resource @a compiler for the given resource @a type and @a version.
	void register_compiler(StringId64 type, u32 version, CompileFunction compiler);

	// Returns the version of the compiler for @a type.
	u32 version(StringId64 type);
};

int main_data_compiler(int argc, char** argv);

} // namespace crown
