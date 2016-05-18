/*
 * Copyright (c) 2012-2016 Daniele Bartolini and individual contributors.
 * License: https://github.com/taylor001/crown/blob/master/LICENSE
 */

#pragma once

#include "compile_options.h"
#include "container_types.h"
#include "disk_filesystem.h"

namespace crown
{
class BundleCompiler
{
	typedef void (*CompileFunction)(const char* path, CompileOptions& opts);

	struct ResourceTypeData
	{
		u32 version;
		CompileFunction compiler;
	};

	DiskFilesystem _source_fs;
	SortMap<StringId64, ResourceTypeData> _compilers;
	Vector<DynamicString> _files;
	Vector<DynamicString> _globs;

	void compile(StringId64 type, const char* path, CompileOptions& opts);
	void scan_source_dir(const char* path);
	bool compile(DiskFilesystem& bundle_fs, const char* type, const char* name, const char* platform);

public:

	BundleCompiler();

	/// Scans @a source_dir for resources.
	void scan(const char* source_dir);

	/// Compiles all the resources found in @a source_dir and puts them in @a bundle_dir.
	/// Returns true on success, false otherwise.
	bool compile(const char* bundle_dir, const char* platform);

	/// Registers the resource @a compiler for the given resource @a type and @a version.
	void register_compiler(StringId64 type, u32 version, CompileFunction compiler);

	// Returns the version of the compiler for @a type.
	u32 version(StringId64 type);
};

} // namespace crown
