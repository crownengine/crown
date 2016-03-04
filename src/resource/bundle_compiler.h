/*
 * Copyright (c) 2012-2016 Daniele Bartolini and individual contributors.
 * License: https://github.com/taylor001/crown/blob/master/LICENSE
 */

#pragma once

#include "disk_filesystem.h"
#include "container_types.h"
#include "compile_options.h"

namespace crown
{
class BundleCompiler
{
	typedef void (*CompileFunction)(const char* path, CompileOptions& opts);

	DiskFilesystem _source_fs;
	DiskFilesystem _bundle_fs;

	struct ResourceTypeData
	{
		u32 version;
		CompileFunction compiler;
	};

	SortMap<StringId64, ResourceTypeData> _compilers;
	Vector<DynamicString> _files;
	Vector<DynamicString> _globs;

	void register_resource_compiler(StringId64 type, u32 version, CompileFunction compiler);
	void compile(StringId64 type, const char* path, CompileOptions& opts);
	// Returns the version of the compiler for @a type.
	u32 version(StringId64 type);

public:

	BundleCompiler(const char* source_dir, const char* bundle_dir);

	bool compile(const char* type, const char* name, const char* platform);

	/// Compiles all the resources found in @a source_dir and puts them in @a bundle_dir.
	/// Returns true on success, false otherwise.
	bool compile_all(const char* platform);

	/// Scans the source directory for resources.
	void scan_source_dir(const char* path);
};

} // namespace crown
