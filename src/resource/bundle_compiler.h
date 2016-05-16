/*
 * Copyright (c) 2012-2016 Daniele Bartolini and individual contributors.
 * License: https://github.com/taylor001/crown/blob/master/LICENSE
 */

#pragma once

#include "compile_options.h"
#include "console_server.h"
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
	DiskFilesystem _bundle_fs;
	SortMap<StringId64, ResourceTypeData> _compilers;
	Vector<DynamicString> _files;
	Vector<DynamicString> _globs;
	ConsoleServer* _console_server;

	void compile(StringId64 type, const char* path, CompileOptions& opts);
	// Returns the version of the compiler for @a type.
	u32 version(StringId64 type);

public:

	BundleCompiler(const char* source_dir, const char* bundle_dir, ConsoleServer& cs);

	/// Runs the compiler. If @a server is true, it listens for compile
	/// requests on port CROWN_DEFAULT_SERVER_PORT.
	bool run(bool server);

	/// Compiles the resource (@a type, @a name) for the given @a platform.
	/// Returns true on success, false otherwise.
	bool compile(const char* type, const char* name, const char* platform);

	/// Compiles all the resources found in @a source_dir and puts them in @a bundle_dir.
	/// Returns true on success, false otherwise.
	bool compile_all(const char* platform);

	/// Scans the source directory for resources.
	void scan_source_dir(const char* path);

	/// Registers the resource @a compiler for the given resource @a type and @a version.
	void register_compiler(StringId64 type, u32 version, CompileFunction compiler);
};

} // namespace crown
