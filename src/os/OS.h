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
#include "Str.h"
#include "List.h"

namespace crown
{

/**
	OS-specific functions and parameters.
*/
namespace os
{

//-----------------------------------------------------------------------------
#ifdef LINUX
const size_t	MAX_OS_PATH_LENGTH = 1024;
const char		PATH_SEPARATOR = '/';
#endif

#ifdef WINDOWS
const size_t	MAX_OS_PATH_LENGTH = 1024;
const char		PATH_SEPARATOR = '\\';
#endif

//-----------------------------------------------------------------------------
void			printf(const char* string, ...);				//!< Printf wrapper
void			vprintf(const char* string, va_list arg);		//!< VPrintf wrapper

void			log_debug(const char* string, va_list arg);		//!< Print debug message
void			log_error(const char* string, va_list arg);		//!< Print error message
void			log_warning(const char* string, va_list arg);	//!< Print warning message
void			log_info(const char* string, va_list arg);		//!< Print info message

bool			exists(const char* path);		//!< Returns whether the path is a file or directory on the disk

bool			is_dir(const char* path);		//!< Returns whether the path is a directory. (May not resolve symlinks.)
bool			is_reg(const char* path);		//!< Returns whether the path is a regular file. (May not resolve symlinks.)

bool			mknod(const char* path);		//! Creates a regular file. Returns true if success, false if not
bool			unlink(const char* path);		//! Deletes a regular file. Returns true if success, false if not
bool			mkdir(const char* path);		//! Creates a directory. Returns true if success, false if not
bool			rmdir(const char* path);		//! Deletes a directory. Returns true if success, false if not

const char*		get_cwd();						//! Fills ret with the path of the current working directory. Returns true if success, false if not 
const char*		get_home();						//! Fills ret with the path of the user home directory
const char*		get_env(const char* env);		//! Returns the content of the 'env' environment variable or the empty string

bool			ls(const char* path, List<Str>& fileList);	//! Returns the list of filenames in a directory.

//-----------------------------------------------------------------------------
void			init_os();

bool			create_render_window(uint x, uint y, uint width, uint height, bool fullscreen);
bool			destroy_render_window();
void			swap_buffers();

void			event_loop();

void			init_keyboard();
void			init_mouse();
void			init_touch();

void			hide_cursor();
void			show_cursor();

//-----------------------------------------------------------------------------
enum OSEventType
{
	OSET_NONE		= 0,
	OSET_KEYBOARD	= 1,
	OSET_MOUSE		= 2,
	OSET_TOUCH		= 3
};

enum OSMouseEventType
{
	OSMET_LEFT_PRESSED		= 0,
	OSMET_MIDDLE_PRESSED	= 1,
	OSMET_RIGHT_PRESSED		= 2,
	OSMET_LEFT_RELEASED		= 3,
	OSMET_MIDDLE_RELEASED	= 4,
	OSMET_RIGHT_RELEASED	= 5,
	OSMET_CURSOR_MOVED		= 6
};

struct OSEvent
{
	OSEventType		type;
	int				data_a;
	int				data_b;
	int				data_c;
	int				data_d;
};

//! Pushes @a event into @a event_queue
void				push_event(OSEventType type, int data_a, int data_b, int data_c, int data_d);

//! Returns the event on top of the @a event_queue	
OSEvent&			pop_event();

} // namespace os

} // namespace crown

