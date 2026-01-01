/*
 * Copyright (c) 2012-2026 Daniele Bartolini et al.
 * SPDX-License-Identifier: MIT
 */

#pragma once

#include "core/containers/types.h"
#include "core/strings/types.h"
#include "core/types.h"

/// @defgroup OS OS
/// @ingroup Core
namespace crown
{
/// Holds information about a file.
///
/// @ingroup OS
struct Stat
{
	enum FileType
	{
		REGULAR,
		DIRECTORY,
		NO_ENTRY
	} file_type;

	u64 size;  ///< Size in bytes.
	u64 mtime; ///< Last modified time in nanoseconds (actual precision depends on underlying filesystem).
};

/// Result from os::delete_file() or os::delete_directory().
///
/// @a ingroup OS
struct DeleteResult
{
	enum
	{
		SUCCESS,  ///<
		NO_ENTRY, ///< Entry does not exist.
		UNKNOWN   ///< Unknown error.
	} error;
};

/// Result from os::create_directory().
///
/// @a ingroup OS
struct CreateResult
{
	enum
	{
		SUCCESS,        ///<
		ALREADY_EXISTS, ///< The entry already exists.
		UNKNOWN         ///< Unknown error.
	} error;
};

/// File access flags to be used with os::access().
///
/// @ingroup OS
struct AccessFlags
{
	enum Enum
	{
		EXISTS  = 0x0,
		EXECUTE = 0x1,
		WRITE   = 0x2,
		READ    = 0x4
	};
};

/// Result from os::rename().
///
/// @a ingroup OS
struct RenameResult
{
	enum
	{
		SUCCESS, ///<
		UNKNOWN  ///< Unknown error.
	} error;
};

/// Operating system functions.
///
/// @ingroup OS
namespace os
{
	/// Suspends execution for @a ms milliseconds.
	void sleep(u32 ms);

	/// Opens the library at @a path.
	void *library_open(const char *path);

	/// Closes a @a library previously opened by library_open.
	void library_close(void *library);

	/// Returns a pointer to the symbol @a name in the given @a library.
	void *library_symbol(void *library, const char *name);

	/// Logs the message @a msg.
	void log(const char *msg);

#if CROWN_PLATFORM_POSIX
	/// Returns the status of the file descriptor @a fd.
	void stat(Stat &st, int fd);
#endif

	/// Returns the status of the file at @a path.
	void stat(Stat &st, const char *path);

	/// Deletes the file at @a path.
	DeleteResult delete_file(const char *path);

	/// Creates a directory @a path.
	CreateResult create_directory(const char *path);

	/// Deletes the directory @a path.
	/// The directory must be empty.
	DeleteResult delete_directory(const char *path);

	/// Returns the list of @a files at the given @a path.
	void list_files(const char *path, Vector<DynamicString> &files);

	/// Returns the current working directory.
	const char *getcwd(char *buf, u32 size);

	/// Sets the current working directory.
	void setcwd(const char *cwd);

	/// Returns the value of the environment variable @a name.
	const char *getenv(const char *name);

	/// Sets the environment variable @a name to @a value. If the variable
	/// does not exist it creates it. Returns 0 on success, otherwise it returns -1.
	s32 setenv(const char *name, const char *value);

	///
	s32 access(const char *path, u32 flags);

	///
	RenameResult rename(const char *old_name, const char *new_name);

} // namespace os

} // namespace crown
