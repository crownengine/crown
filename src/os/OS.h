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

namespace Crown
{

/**
	OS-specific functions and parameters.
*/
namespace OS
{

//-----------------------------------------------------------------------------
#ifdef LINUX
const char		PATH_SEPARATOR = '/';
#endif

#ifdef WINDOWS
const char		PATH_SEPARATOR = '\\';
#endif

void			Printf(const char* string, ...);				//!< Printf wrapper
void			Vprintf(const char* string, va_list arg);		//!< VPrintf wrapper

void			LogDebug(const char* string, va_list arg);		//!< Print debug message
void			LogError(const char* string, va_list arg);		//!< Print error message
void			LogWarning(const char* string, va_list arg);	//!< Print warning message
void			LogInfo(const char* string, va_list arg);		//!< Print info message

bool			Exists(const char* path);		//!< Returns whether the path is a file or directory on the disk

bool			IsDir(const char* path);		//!< Returns whether the path is a directory. (May not resolve symlinks.)
bool			IsReg(const char* path);		//!< Returns whether the path is a regular file. (May not resolve symlinks.)

bool			Mknod(const char* path);		//! Creates a regular file. Returns true if success, false if not
bool			Unlink(const char* path);		//! Deletes a regular file. Returns true if success, false if not
bool			Mkdir(const char* path);		//! Creates a directory. Returns true if success, false if not
bool			Rmdir(const char* path);		//! Deletes a directory. Returns true if success, false if not

const char*		GetCWD();						//! Fills ret with the path of the current working directory. Returns true if success, false if not 
const char*		GetHome();						//! Fills ret with the path of the user home directory
const char*		GetEnv(const char* env);		//! Returns the content of the 'env' environment variable or the empty string

bool			Ls(const char* path, List<Str>& fileList);	//! Returns the list of filenames in a directory.

//-----------------------------------------------------------------------------
enum OSEventType
{
	OSET_NONE		= 0,
	OSET_KEYBOARD	= 1,
	OSET_MOUSE		= 2,
	OSET_TOUCH		= 3
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

} // namespace OS

} // namespace Crown

