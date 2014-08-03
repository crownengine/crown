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

#include <jni.h>
#include <android/log.h>
#include <cstdarg>
#include <cstdio>
#include <cstdlib>
#include <dirent.h>
#include <dlfcn.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>

#include "os.h"
#include "assert.h"
#include "string_utils.h"
#include "os_types.h"

namespace crown
{
namespace os
{

extern timespec g_base_time;

//-----------------------------------------------------------------------------
void log_debug(const char* string, va_list arg)
{
	__android_log_vprint(ANDROID_LOG_DEBUG, "crown", string, arg);
}

//-----------------------------------------------------------------------------
void log_error(const char* string, va_list arg)
{
	__android_log_vprint(ANDROID_LOG_ERROR, "crown", string, arg);
}

//-----------------------------------------------------------------------------
void log_warning(const char* string, va_list arg)
{
	__android_log_vprint(ANDROID_LOG_WARN, "crown", string, arg);
}

//-----------------------------------------------------------------------------
void log_info(const char* string, va_list arg)
{
	__android_log_vprint(ANDROID_LOG_INFO, "crown", string, arg);
}

//-----------------------------------------------------------------------------
void init_os()
{
	// Initilize the base time
	clock_gettime(CLOCK_MONOTONIC, &g_base_time);
}

} // namespace os
} // namespace crown
