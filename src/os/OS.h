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

#include "Str.h"
#include "List.h"

namespace Crown
{

/**
	OS-specific functions and parameters.
*/
class OS
{

public:

	static void			Printf(const char* string, ...);				//!< Printf wrapper
	static void			Vprintf(const char* string, va_list arg);		//!< VPrintf wrapper

	static void			LogDebug(const char* string, va_list arg);		//!< Print debug message
	static void			LogError(const char* string, va_list arg);		//!< Print error message
	static void			LogWarning(const char* string, va_list arg);	//!< Print warning message
	static void			LogInfo(const char* string, va_list arg);		//!< Print info message

	static bool			Exists(const char* path);		//!< Returns whether the path is a file or directory on the disk

	static bool			IsDir(const char* path);		//!< Returns whether the path is a directory. (May not resolve symlinks.)
	static bool			IsReg(const char* path);		//!< Returns whether the path is a regular file. (May not resolve symlinks.)

	static bool			Mknod(const char* path);		//! Creates a regular file. Returns true if success, false if not
	static bool			Unlink(const char* path);		//! Deletes a regular file. Returns true if success, false if not
	static bool			Mkdir(const char* path);		//! Creates a directory. Returns true if success, false if not
	static bool			Rmdir(const char* path);		//! Deletes a directory. Returns true if success, false if not

	static const char*	GetCWD();						//! Fills ret with the path of the current working directory. Returns true if success, false if not 
	static const char*	GetHome();						//! Fills ret with the path of the user home directory
	static const char*	GetEnv(const char* env);		//! Returns the content of the 'env' environment variable or the empty string

	static bool			Ls(const char* path, List<Str>& fileList);	//! Returns the list of filenames in a directory.

	static const char	PATH_SEPARATOR;							//! OS-specific path separator

private:

	// Disable construction
	OS();
};

} // namespace Crown

