/*
 * Copyright (c) 2012-2018 Daniele Bartolini and individual contributors.
 * License: https://github.com/dbartolini/crown/blob/master/LICENSE
 */

#pragma once

#include "core/containers/types.h"
#include "core/filesystem/types.h"
#include "core/strings/dynamic_string.h"
#include "core/strings/types.h"
#include "resource/types.h"
#include <stdarg.h>

#define DATA_COMPILER_ASSERT(condition, opts, msg, ...) \
	do                                                  \
	{                                                   \
		if (!(condition))                               \
			opts.error(msg, ## __VA_ARGS__);            \
	} while(0)

#define DATA_COMPILER_ASSERT_RESOURCE_EXISTS(type, name, opts) \
	DATA_COMPILER_ASSERT(opts.resource_exists(type, name)      \
		, opts                                                 \
		, "Resource not found: '%s.%s'"                        \
		, name                                                 \
		, type                                                 \
		)

#define DATA_COMPILER_ASSERT_FILE_EXISTS(name, opts) \
	DATA_COMPILER_ASSERT(opts.file_exists(name)      \
		, opts                                       \
		, "File not found: '%s'"                     \
		, name                                       \
		)

namespace crown
{
struct CompileOptions
{
	DataCompiler& _data_compiler;
	Filesystem& _data_filesystem;
	DynamicString _source_path;
	Buffer& _output;
	const char* _platform;
	Vector<DynamicString> _dependencies;

	///
	CompileOptions(DataCompiler& dc, Filesystem& data_filesystem, DynamicString& source_path, Buffer& output, const char* platform);

	///
	void error(const char* msg, va_list args);

	///
	void error(const char* msg, ...);

	///
	const char* source_path();

	///
	bool file_exists(const char* path);

	///
	bool resource_exists(const char* type, const char* name);

	///
	Buffer read_temporary(const char* path);

	///
	void write_temporary(const char* path, const char* data, u32 size);

	///
	void write_temporary(const char* path, const Buffer& data);

	///
	Buffer read();

	///
	Buffer read(const char* path);

	///
	void get_absolute_path(const char* path, DynamicString& abs);

	///
	void get_temporary_path(const char* suffix, DynamicString& abs);

	///
	void delete_file(const char* path);

	///
	void write(const void* data, u32 size);

	///
	template <typename T>
	void write(const T& data)
	{
		write(&data, sizeof(data));
	}

	///
	void write(const Buffer& data);

	///
	const char* platform() const;

	///
	const Vector<DynamicString>& dependencies() const;

	///
	void add_dependency(const char* path);

	///
	int run_external_compiler(const char* const* argv, StringStream& output);
};

} // namespace crown
