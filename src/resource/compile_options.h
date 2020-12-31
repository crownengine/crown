/*
 * Copyright (c) 2012-2021 Daniele Bartolini et al.
 * License: https://github.com/dbartolini/crown/blob/master/LICENSE
 */

#pragma once

#include "config.h"

#if CROWN_CAN_COMPILE

#include "core/containers/types.h"
#include "core/filesystem/reader_writer.h"
#include "core/filesystem/types.h"
#include "core/os.h"
#include "core/process.h"
#include "core/strings/dynamic_string.h"
#include "core/strings/string_stream.h"
#include "core/strings/types.h"
#include "resource/resource_id.h"
#include "resource/types.h"
#include <stdarg.h>

#define DATA_COMPILER_ASSERT(condition, opts, msg, ...) \
	do                                                  \
	{                                                   \
		if (!(condition))                               \
		{                                               \
			opts.error(msg, ## __VA_ARGS__);            \
			return -1;                                  \
		}                                               \
	} while(0)

#define DATA_COMPILER_ENSURE(condition, opts)             \
	DATA_COMPILER_ASSERT(condition                        \
		, opts                                            \
		, "DATA_COMPILER_ENSURE(" # condition ") failed." \
		)

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
	File& _file;
	BinaryWriter _binary_writer;
	HashMap<DynamicString, u32>& _new_dependencies;
	HashMap<DynamicString, u32>& _new_requirements;
	DataCompiler& _data_compiler;
	Filesystem& _data_filesystem;
	DynamicString _source_path;
	const char* _platform;
	ResourceId _resource_id;

	///
	CompileOptions(File& output
		, HashMap<DynamicString, u32>& new_dependencies
		, HashMap<DynamicString, u32>& new_requirements
		, DataCompiler& dc
		, Filesystem& data_filesystem
		, ResourceId res_id
		, const DynamicString& source_path
		, const char* platform
		);

	///
	CompileOptions(const CompileOptions&) = delete;

	///
	CompileOptions& operator=(const CompileOptions&) = delete;

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
	Buffer read_all(File* file);

	///
	Buffer read_temporary(const char* path);

	///
	void write_temporary(const char* path, const char* data, u32 size);

	///
	void write_temporary(const char* path, const Buffer& data);

	/// Reads the data at @a path and returns it.
	/// It also registers @a path as a dependency.
	Buffer read(const char* path);

	/// Reads the source data and returns it.
	/// It also registers the source path as a dependency.
	Buffer read();

	/// Registers @a path as dependency without reading anything.
	void fake_read(const char* path);

	///
	void add_requirement(const char* type, const char* name);

	///
	void absolute_path(DynamicString& abs, const char* path);

	///
	void temporary_path(DynamicString& abs, const char* suffix);

	///
	DeleteResult delete_file(const char* path);

	///
	void align(const u32 align);

	///
	void write(const void* data, u32 size);

	///
	template <typename T>
	void write(const T& data);

	///
	void write(const Buffer& data);

	///
	const char* platform() const;

	/// Returns the first path with executable permissions or NULL if none found.
	const char* exe_path(const char* const* paths, u32 num);

	///
	void read_output(StringStream& ss, Process& pr);
};

} // namespace crown

#endif // CROWN_CAN_COMPILE
