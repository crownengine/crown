/*
 * Copyright (c) 2012-2016 Daniele Bartolini and individual contributors.
 * License: https://github.com/taylor001/crown/blob/master/LICENSE
 */

#pragma once

#include "compiler_types.h"
#include "container_types.h"
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

	FilesystemDisk _source_fs;
	Map<DynamicString, DynamicString> _source_dirs;
	SortMap<StringId64, ResourceTypeData> _compilers;
	Vector<DynamicString> _files;
	Vector<DynamicString> _globs;

	void compile(StringId64 type, const char* path, CompileOptions& opts);
	void scan_source_dir(const char* prefix, const char* path);
	bool compile(FilesystemDisk& bundle_fs, const char* type, const char* name, const char* platform);

public:

	DataCompiler();

	void map_source_dir(const char* name, const char* source_dir);
	void source_dir(const char* resource_name, DynamicString& source_dir);

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

} // namespace crown
