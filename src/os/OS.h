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

#include "Config.h"
#include "List.h"
#include <cstdarg>
#include "Types.h"

namespace crown
{

//-----------------------------------------------------------------------------
#ifdef LINUX
const size_t	MAX_PATH_LENGTH = 1024;
const char		PATH_SEPARATOR = '/';
#endif

#ifdef WINDOWS
const size_t	MAX_PATH_LENGTH = 1024;
const char		PATH_SEPARATOR = '\\';
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
bool			exists(const char* path);		//!< Returns whether the path is a file or directory on the disk

bool			is_dir(const char* path);		//!< Returns whether the path is a directory. (May not resolve symlinks.)
bool			is_reg(const char* path);		//!< Returns whether the path is a regular file. (May not resolve symlinks.)

bool			mknod(const char* path);		//! Creates a regular file. Returns true if success, false if not
bool			unlink(const char* path);		//! Deletes a regular file. Returns true if success, false if not
bool			mkdir(const char* path);		//! Creates a directory. Returns true if success, false if not
bool			rmdir(const char* path);		//! Deletes a directory. Returns true if success, false if not

//-----------------------------------------------------------------------------
// OS ambient variables
//-----------------------------------------------------------------------------
const char*		get_cwd();						//! Fills ret with the path of the current working directory. Returns true if success, false if not 
const char*		get_home();						//! Fills ret with the path of the user home directory
const char*		get_env(const char* env);		//! Returns the content of the 'env' environment variable or the empty string

//bool			ls(const char* path, List<Str>& fileList);	//! Returns the list of filenames in a directory.

//-----------------------------------------------------------------------------
// Render window and input management
//-----------------------------------------------------------------------------
void			init_os();

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

} // namespace os

//-----------------------------------------------------------------------------
// Events
//-----------------------------------------------------------------------------
enum OsEventType
{
	OSET_NONE				= 0,

	OSET_KEY_PRESS			= 1,
	OSET_KEY_RELEASE		= 2,

	OSET_BUTTON_PRESS		= 3,
	OSET_BUTTON_RELEASE		= 4,
	OSET_MOTION_NOTIFY		= 5,
	OSET_TOUCH_DOWN			= 6,
	OSET_TOUCH_MOVE			= 7,
	OSET_TOUCH_UP			= 8,
	OSET_ACCELEROMETER		= 9
};

union OsEventParameter
{
	int32_t int_value;
	float	float_value;
};

struct OsEvent
{
	OsEventType			type;
	OsEventParameter	data_a;
	OsEventParameter	data_b;
	OsEventParameter	data_c;
	OsEventParameter	data_d;
};

/// Pushes the event @a type along with its parameters into the os' event queue.
void			push_event(OsEventType type, OsEventParameter data_a, OsEventParameter data_b, OsEventParameter data_c, OsEventParameter data_d);

/// Returns and pops the first event in the os' event queue.
OsEvent&		pop_event();

} // namespace crown

