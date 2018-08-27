/*
 * Copyright (c) 2012-2018 Daniele Bartolini and individual contributors.
 * License: https://github.com/dbartolini/crown/blob/master/LICENSE
 */

#pragma once

#include "core/containers/types.h"
#include "core/filesystem/file_monitor.h"
#include "core/filesystem/filesystem_disk.h"
#include "device/console_server.h"
#include "device/device_options.h"
#include "resource/types.h"
#include <setjmp.h>

namespace crown
{
/// Compiles source data into binary.
///
/// @ingroup Resource
struct DataCompiler
{
	typedef void (*CompileFunction)(CompileOptions& opts);

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
	Map<DynamicString, DynamicString> _data_index;
	FileMonitor _file_monitor;
	jmp_buf _jmpbuf;

	void add_file(const char* path);
	void add_tree(const char* path);
	void remove_file(const char* path);
	void remove_tree(const char* path);
	void scan_source_dir(const char* prefix, const char* path);

	void filemonitor_callback(FileMonitorEvent::Enum fme, bool is_dir, const char* path, const char* path_renamed);
	static void filemonitor_callback(void* thiz, FileMonitorEvent::Enum fme, bool is_dir, const char* path_original, const char* path_modified);

	///
	DataCompiler(ConsoleServer& cs);

	///
	~DataCompiler();

	///
	void map_source_dir(const char* name, const char* source_dir);

	///
	void source_dir(const char* resource_name, DynamicString& source_dir);

	/// Adds a @a glob pattern to ignore when scanning the source directory.
	void add_ignore_glob(const char* glob);

	/// Scans source directory for resources.
	void scan();

	/// Compiles all the resources found in the source directory and puts them in @a data_dir.
	/// Returns true on success, false otherwise.
	bool compile(const char* data_dir, const char* platform);

	/// Registers the resource @a compiler for the given resource @a type and @a version.
	void register_compiler(StringId64 type, u32 version, CompileFunction compiler);

	/// Returns whether there is a compiler for the resource @a type.
	bool can_compile(StringId64 type);

	/// Returns the version of the compiler for @a type or COMPILER_NOT_FOUND if no compiler
	/// is found.
	u32 version(StringId64 type);

	///
	void error(const char* msg, va_list args);

	static const u32 COMPILER_NOT_FOUND = UINT32_MAX;
};

int main_data_compiler(const DeviceOptions& opts);

} // namespace crown
