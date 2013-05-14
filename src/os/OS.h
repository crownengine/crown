/*
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

#ifdef LINUX
	#include <pthread.h>
#endif

namespace crown
{

/// OS-specific functions and parameters.
namespace os
{

//-----------------------------------------------------------------------------
#ifdef LINUX
const size_t	MAX_PATH_LENGTH = 1024;
const char		PATH_SEPARATOR = '/';

const size_t	MAX_EVENTS = 512;

const size_t	MAX_THREADS = 16;
const size_t	MAX_MUTEXES = 16;

struct OSThread
{
	pthread_t	thread;
	const char*	name;
};

struct OSMutex
{
	pthread_mutex_t mutex;
};

struct OSCond
{
	pthread_cond_t cond;
};

#endif

#ifdef WINDOWS
const size_t	MAX_PATH_LENGTH = 1024;
const char		PATH_SEPARATOR = '\\';

const size_t	MAX_EVENTS = 512;

const size_t	MAX_THREADS = 16;
const size_t	MAX_MUTEXES = 16;
#endif

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

bool			create_render_window(uint32_t x, uint32_t y, uint32_t width, uint32_t height, bool fullscreen);
bool			destroy_render_window();
void			get_render_window_metrics(uint32_t& width, uint32_t& height);
void			swap_buffers();

void			event_loop();

void			init_input();
void			get_cursor_xy(int32_t& x, int32_t& y);
void			set_cursor_xy(int32_t x, int32_t y);

void			hide_cursor();
void			show_cursor();

//-----------------------------------------------------------------------------
// Timing
//-----------------------------------------------------------------------------
uint64_t		milliseconds();
uint64_t		microseconds();

//-----------------------------------------------------------------------------
// Events
//-----------------------------------------------------------------------------
enum OSEventType
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

union OSEventParameter
{
	int32_t int_value;
	float	float_value;
};

struct OSEvent
{
	OSEventType			type;
	OSEventParameter	data_a;
	OSEventParameter	data_b;
	OSEventParameter	data_c;
	OSEventParameter	data_d;
};

/// Pushes the event @type along with its parameters into the os' event queue.
void			push_event(OSEventType type, OSEventParameter data_a, OSEventParameter data_b, OSEventParameter data_c, OSEventParameter data_d);


/// Returns and pops the first event in the os' event queue.
OSEvent&		pop_event();

//-----------------------------------------------------------------------------
// Dynamic libraries
//-----------------------------------------------------------------------------
void*			open_library(const char* path);
void			close_library(void* library);
void*			lookup_symbol(void* library, const char* name);

//-----------------------------------------------------------------------------
// Threads
//-----------------------------------------------------------------------------
typedef			void* (*ThreadFunction)(void*);

void			thread_create(ThreadFunction f, void* params, OSThread* thread, const char* name);
void			thread_join(OSThread* thread);
void			thread_detach(OSThread* thread);

void			mutex_create(OSMutex* mutex);
void			mutex_destroy(OSMutex* mutex);
void			mutex_lock(OSMutex* mutex);
void			mutex_unlock(OSMutex* mutex);
void			cond_create(OSCond* cond);
void			cond_destroy(OSCond* cond);
void			cond_signal(OSCond* cond);
void			cond_wait(OSCond* cond, OSMutex* mutex);

} // namespace os
} // namespace crown

