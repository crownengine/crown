/*
 * Copyright (c) 2012-2021 Daniele Bartolini et al.
 * License: https://github.com/dbartolini/crown/blob/master/LICENSE
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
	u64 mtime; ///< Last modified time.
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

/// Operating system functions.
///
/// @ingroup OS
namespace os
{
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

#if CROWN_PLATFORM_POSIX
	/// Returns information about @a fd.
	void stat(Stat& info, int fd);
#endif

	/// Returns information about @a path.
	void stat(Stat& info, const char* path);

	/// Deletes the file at @a path.
	DeleteResult delete_file(const char* path);

	/// Creates a directory @a path.
	CreateResult create_directory(const char* path);

	/// Deletes the directory @a path.
	/// The directory must be empty.
	DeleteResult delete_directory(const char* path);

	/// Returns the list of @a files at the given @a path.
	void list_files(const char* path, Vector<DynamicString>& files);

	/// Returns the current working directory.
	const char* getcwd(char* buf, u32 size);

	/// Returns the value of the environment variable @a name.
	const char* getenv(const char* name);

	///
	s32 access(const char* path, u32 flags);

} // namespace os

} // namespace crown
