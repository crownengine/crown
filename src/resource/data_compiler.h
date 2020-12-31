/*
 * Copyright (c) 2012-2021 Daniele Bartolini et al.
 * License: https://github.com/dbartolini/crown/blob/master/LICENSE
 */

#pragma once

#include "core/containers/types.h"
#include "core/filesystem/file_monitor.h"
#include "core/filesystem/filesystem_disk.h"
#include "core/guid.h"
#include "device/console_server.h"
#include "device/device_options.h"
#include "resource/resource_id.h"
#include "resource/types.h"

namespace crown
{
/// Source data index.
/// Associates a path on disk with its metadata.
///
/// @ingroup Resource
struct SourceIndex
{
	HashMap<DynamicString, Stat> _paths;

	///
	SourceIndex();

	///
	void scan_directory(FilesystemDisk& fs, const char* prefix, const char* directory);

	/// Scans all directories defined by @a source_dirs.
	/// @a source_dirs maps a relative directory name to its absolute parent
	/// directory.
	void scan(const HashMap<DynamicString, DynamicString>& source_dirs);
};

/// Compiles source data into binary.
///
/// @ingroup Resource
struct DataCompiler
{
	typedef s32 (*CompileFunction)(CompileOptions& opts);

	struct ResourceTypeData
	{
		u32 version;
		CompileFunction compiler;
	};

	const DeviceOptions* _options;
	ConsoleServer* _console_server;
	FilesystemDisk _source_fs;
	HashMap<DynamicString, DynamicString> _source_dirs;
	HashMap<DynamicString, ResourceTypeData> _compilers;
	Vector<DynamicString> _globs;
	HashMap<StringId64, DynamicString> _data_index;
	HashMap<StringId64, u64> _data_mtimes;
	HashMap<StringId64, HashMap<DynamicString, u32> > _data_dependencies;
	HashMap<StringId64, HashMap<DynamicString, u32> > _data_requirements;
	HashMap<DynamicString, u32> _data_versions;
	FileMonitor _file_monitor;
	SourceIndex _source_index;
	HashMap<StringId64, u32> _data_revisions;
	HashMap<Guid, u32> _client_revisions;
	u32 _revision;

	void add_file(const char* path);
	void remove_file(const char* path);
	void add_tree(const char* path);
	void remove_tree(const char* path);
	void remove_file_or_tree(const char* path);

	void file_monitor_callback(FileMonitorEvent::Enum fme, bool is_dir, const char* path, const char* path_renamed);
	static void file_monitor_callback(void* thiz, FileMonitorEvent::Enum fme, bool is_dir, const char* path_original, const char* path_modified);

	///
	DataCompiler(const DeviceOptions& opts, ConsoleServer& cs);

	///
	~DataCompiler();

	///
	void map_source_dir(const char* name, const char* source_dir);

	///
	void source_dir(const char* resource_name, DynamicString& source_dir);

	/// Adds a @a glob pattern to ignore when scanning the source directory.
	void add_ignore_glob(const char* glob);

	/// Scans the source directory for resources and restores the state of the compiler from
	/// the previous run (if any).
	void scan_and_restore(const char* data_dir);

	/// Saves to disk the state of the compiler.
	void save(const char* data_dir);

	/// Compiles all the resources found in the source directory and puts them in @a data_dir.
	/// Returns true on success, false otherwise.
	bool compile(const char* data_dir, const char* platform);

	/// Registers the resource @a compiler for the given resource @a type and @a version.
	void register_compiler(const char* type, u32 version, CompileFunction compiler);

	/// Returns whether there is a compiler for the resource @a type.
	bool can_compile(const char* type);

	/// Returns the version of the compiler for @a type or COMPILER_NOT_FOUND if no compiler
	/// is found.
	u32 data_version(const char* type);

	///
	u32 data_version_stored(const char* type);

	/// Returns whether any dependency of @a path, including itself, has changed
	/// since last call to compile().
	bool dependency_changed(const DynamicString& path, ResourceId id, u64 mtime);

	/// Returns whether the data version for @a path or any of its dependencies
	/// has changed since last call to compile().
	bool version_changed(const DynamicString& path, ResourceId id);

	/// Returns whether the @a path should be ignored because
	/// it matches a pattern from the CROWN_DATAIGNORE file or
	/// by other means.
	bool path_matches_ignore_glob(const char* path);

	/// Returns whether the @a path is special, hence it must
	/// be treated differently by some code somewhere. Sigh.
	bool path_is_special(const char* path);

	///
	void error(const char* msg, va_list args);

	static const u32 COMPILER_NOT_FOUND = UINT32_MAX;
};

int main_data_compiler(const DeviceOptions& opts);

} // namespace crown
