/*
Copyright (c) 2013 Daniele Bartolini, Michele Rossi
Copyright (c) 2012 Daniele Bartolini, Simone Boscaratto

Permission is hereby granted, free of charge, to any person
obtaining a copy of this software and associated documentation
files (the "Software"), to deal in the Software without
restriction, including without limitation the rights to use,
copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the
Software is furnished to do so, subject to the following
conditions:

The above copyright notice and this permission notice shall be
included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
OTHER DEALINGS IN THE SOFTWARE.
*/

#pragma once

#include <cstdarg>

#include "Config.h"
#include "Types.h"
#include "Vector.h"
#include "DynamicString.h"
#include "EventBuffer.h"

namespace crown
{

//-----------------------------------------------------------------------------
#if defined(LINUX)
	const size_t	MAX_PATH_LENGTH = 1024;
	const char		PATH_SEPARATOR = '/';
#elif defined(WINDOWS)
	const size_t	MAX_PATH_LENGTH = 1024;
	const char		PATH_SEPARATOR = '\\';

	#define snprintf _snprintf

	#define Thread os::Thread

	#undef MK_SHIFT
	#undef MK_ALT
#elif defined(ANDROID)
	const size_t	MAX_PATH_LENGTH = 1024;
	const char		PATH_SEPARATOR = '/';
#else
	#error "Oops, invalid platform!"
#endif

namespace os
{
//-----------------------------------------------------------------------------
// Print and log functions
//-----------------------------------------------------------------------------
void			printf(const char* string, ...);				//!< Printf wrapper
void			vprintf(const char* string, va_list arg);		//!< VPrintf wrapper

void			log_debug(const char* string, va_list arg);		//!< Print debug message
void			log_error(const char* string, va_list arg);		//!< Print error message
void			log_warning(const char* string, va_list arg);	//!< Print warning message
void			log_info(const char* string, va_list arg);		//!< Print info message

//-----------------------------------------------------------------------------
// Paths
//-----------------------------------------------------------------------------
bool			is_root_path(const char* path);
bool			is_absolute_path(const char* path);

//-----------------------------------------------------------------------------
// File management
//-----------------------------------------------------------------------------

/// Returns whether the path is a file or directory on the disk
bool			exists(const char* path);

/// Returns whether the path is a directory. (May not resolve symlinks.)
bool			is_directory(const char* path);

/// Returns whether the path is a regular file. (May not resolve symlinks.)
bool			is_file(const char* path);

/// Creates a regular file. Returns true if success, false if not
bool			create_file(const char* path);

/// Deletes a regular file. Returns true if success, false if not
bool			delete_file(const char* path);

/// Creates a directory. Returns true if success, false if not
bool			create_directory(const char* path);

/// Deletes a directory. Returns true if success, false if not
bool			delete_directory(const char* path);

/// Returns the list of @a files in the given @a dir directory. Optionally walks into
/// subdirectories whether @a recursive is true.
/// @note
/// Does not follow symbolic links.
void			list_files(const char* path, Vector<DynamicString>& files);

/// Returns os-dependent path from os-indipendent @a path
const char*		normalize_path(const char* path);

//-----------------------------------------------------------------------------
// OS ambient variables
//-----------------------------------------------------------------------------

/// Fills ret with the path of the current working directory. Returns true if success, false if not 
const char*		get_cwd();

/// Fills ret with the path of the user home directory
const char*		get_home();

/// Returns the content of the 'env' environment variable or the empty string
const char*		get_env(const char* env);

//-----------------------------------------------------------------------------
// Render window and input management
//-----------------------------------------------------------------------------
CE_EXPORT void	init_os();

void			get_cursor_xy(int32_t& x, int32_t& y);
void			set_cursor_xy(int32_t x, int32_t y);

//-----------------------------------------------------------------------------
// Timing
//-----------------------------------------------------------------------------
uint64_t		milliseconds();
uint64_t		microseconds();

//-----------------------------------------------------------------------------
// Dynamic libraries
//-----------------------------------------------------------------------------
void*			open_library(const char* path);
void			close_library(void* library);
void*			lookup_symbol(void* library, const char* name);

//-----------------------------------------------------------------------------
// Process execution
//-----------------------------------------------------------------------------

/// Executes a process.
/// @a args is an array of arguments where:
/// @a args[0] is the path to the program executable,
/// @a args[1, 2, ..., n-1] is a list of arguments to pass to the executable,
/// @a args[n] is NULL.
void			execute_process(const char* args[]);

//-----------------------------------------------------------------------------

} // namespace os

EventBuffer* 	os_event_buffer();

} // namespace crown

