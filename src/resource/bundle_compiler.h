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
public:

	BundleCompiler(const char* source_dir, const char* bundle_dir);

	bool compile(const char* type, const char* name, const char* platform);

	/// Compiles all the resources found in @a source_dir and puts them in @a bundle_dir.
	/// Returns true on success, false otherwise.
	bool compile_all(const char* platform);

	/// Scans the source directory for resources.
	void scan_source_dir(const char* path);

private:

	typedef void (*CompileFunction)(const char* path, CompileOptions& opts);

	void register_resource_compiler(StringId64 type, uint32_t version, CompileFunction compiler);
	void compile(StringId64 type, const char* path, CompileOptions& opts);

	// Returns the version of the compiler for @a type.
	uint32_t version(StringId64 type);

private:

	DiskFilesystem _source_fs;
	DiskFilesystem _bundle_fs;

	struct ResourceTypeData
	{
		uint32_t version;
		CompileFunction compiler;
	};

	SortMap<StringId64, ResourceTypeData> _compilers;
	Vector<DynamicString> _files;
};

namespace bundle_compiler
{
	bool main(bool do_compile, bool do_continue, const char* platform);
} // namespace bundle_compiler

namespace bundle_compiler_globals
{
	/// Creates the global resource compiler.
	void init(const char* source_dir, const char* bundle_dir);

	/// Destroys the global resource compiler.
	void shutdown();

	/// Returns the global resource compiler.
	/// Returns NULL if the compiler is not available on the
	/// running platform.
	BundleCompiler* compiler();
} // namespace bundle_compiler_globals
} // namespace crown
