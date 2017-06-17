/*
 * Copyright (c) 2012-2017 Daniele Bartolini and individual contributors.
 * License: https://github.com/taylor001/crown/blob/master/LICENSE
 */

#pragma once

#include "core/strings/types.h"
#include "core/types.h"

/// @defgroup OS OS
/// @ingroup Core
namespace crown
{
/// Operating system functions.
///
/// @ingroup OS
namespace os
{
	/// Returns the clock time.
	s64 clocktime();

	/// Returns che clock frequency.
	s64 clockfrequency();

	/// Suspends execution for @a ms milliseconds.
	void sleep(u32 ms);

	/// Opens the library at @a path.
	void* library_open(const char* path);

	/// Closes a @a library previously opened by library_open.
	void library_close(void* library);

	/// Returns a pointer to the symbol @a name in the given @a library.
	void* library_symbol(void* library, const char* name);

	/// Logs the message @a msg.
	void log(const char* msg);

	/// Returns whether the @a path exists.
	bool exists(const char* path);

	/// Returns whether @a path is a directory.
	bool is_directory(const char* path);

	/// Returns whether @a path is a regular file.
	bool is_file(const char* path);

	/// Returns the last modification time of @a path.
	u64 mtime(const char* path);

	/// Creates a regular file named @a path.
	void create_file(const char* path);

	/// Deletes the file at @a path.
	void delete_file(const char* path);

	/// Creates a directory named @a path.
	void create_directory(const char* path);

	/// Deletes the directory at @a path.
	void delete_directory(const char* path);

	/// Returns the list of @a files at the given @a path.
	void list_files(const char* path, Vector<DynamicString>& files);

	/// Returns the current working directory.
	const char* getcwd(char* buf, u32 size);

	/// Returns the value of the environment variable @a name.
	const char* getenv(const char* name);

	/// Executes the process described by @a argv and returns its exit code.
	/// It fills @a output with stdout and stderr.
	int execute_process(const char* const* argv, StringStream& output);

} // namespace os

} // namespace crown
