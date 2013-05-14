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

#include <android/log.h>
#include <cstdio>
#include <cstdarg>
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>
#include <cstdlib>
#include <sys/time.h>
#include <time.h>
#include <android/asset_manager_jni.h>
#include <pthread.h>

#include "OS.h"
#include "AndroidOS.h"
#include "Log.h"

namespace crown
{
namespace os
{

extern "C" 
{
	// This is sadly necessary in order to get the asset manager from java...
    JNIEXPORT void JNICALL Java_crown_android_CrownLib_initAssetManager(JNIEnv* env, jobject obj, jobject assetManager);
	JNIEXPORT void JNICALL Java_crown_android_CrownLib_setRenderWindowMetrics(JNIEnv* env, jobject obj, jint width, jint height);
};

static timespec			base_time;
static AAssetManager*	asset_manager = NULL;
static uint32_t			window_width;
static uint32_t			window_height;

//-----------------------------------------------------------------------------
void printf(const char* string, ...)
{
	va_list args;

	va_start(args, string);
	::vprintf(string, args);
	va_end(args);
}

//-----------------------------------------------------------------------------
void vprintf(const char* string, va_list arg)
{
	::vprintf(string, arg);
}

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
bool exists(const char* path)
{
	struct stat dummy;
	return (stat(path, &dummy) == 0);
}

//-----------------------------------------------------------------------------
bool is_dir(const char* path)
{
	struct stat info;
	memset(&info, 0, sizeof(struct stat));
	lstat(path, &info);
	return ((S_ISDIR(info.st_mode)) != 0 && (S_ISLNK(info.st_mode) == 0));
}

//-----------------------------------------------------------------------------
bool is_reg(const char* path)
{
	struct stat info;
	memset(&info, 0, sizeof(struct stat));
	lstat(path, &info);
	return ((S_ISREG(info.st_mode) != 0) && (S_ISLNK(info.st_mode) == 0));
}

//-----------------------------------------------------------------------------
bool mknod(const char* path)
{
	// Permission mask: rw-r--r--
	return ::mknod(path, S_IFREG | S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH, 0) == 0;
}

//-----------------------------------------------------------------------------
bool unlink(const char* path)
{
	return (::unlink(path) == 0);
}

//-----------------------------------------------------------------------------
bool mkdir(const char* path)
{
	// rwxr-xr-x permission mask
	return (::mkdir(path, S_IRWXU | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH) == 0);
}

//-----------------------------------------------------------------------------
bool rmdir(const char* path)
{
	return (::rmdir(path) == 0);
}

//-----------------------------------------------------------------------------
const char* get_cwd()
{
	static char cwdBuf[MAX_PATH_LENGTH];
	if (getcwd(cwdBuf, MAX_PATH_LENGTH) == NULL)
	{
		return string::EMPTY;
	}

	return cwdBuf;
}

//-----------------------------------------------------------------------------
const char* get_home()
{
	char* envHome = NULL;
	envHome = getenv("HOME");

	if (envHome == NULL)
	{
		return string::EMPTY;
	}

	return envHome;
}

//-----------------------------------------------------------------------------
const char* get_env(const char* env)
{
	char* envDevel = NULL;
	envDevel = getenv(env);

	if (envDevel == NULL)
	{
		return string::EMPTY;
	}

	return envDevel;
}

//-----------------------------------------------------------------------------
// bool ls(const char* path, List<Str>& fileList)
// {
// 	DIR *dir;
// 	struct dirent *ent;

// 	dir = opendir(path);

// 	if (dir == NULL)
// 	{
// 		return false;
// 	}

// 	while ((ent = readdir (dir)) != NULL)
// 	{
// 		fileList.push_back(Str(ent->d_name));
// 	}

// 	closedir (dir);

// 	return true;
// }

//-----------------------------------------------------------------------------
void init_os()
{
	// Initilize the base time
	clock_gettime(CLOCK_MONOTONIC, &base_time);
}

//-----------------------------------------------------------------------------
uint64_t milliseconds()
{
	timespec tmp;

	clock_gettime(CLOCK_MONOTONIC, &tmp);

	return (tmp.tv_sec - base_time.tv_sec) * 1000 + (tmp.tv_nsec - base_time.tv_nsec) / 1000000;
}

//-----------------------------------------------------------------------------
uint64_t microseconds()
{
	timespec tmp;
	clock_gettime(CLOCK_MONOTONIC, &tmp);
	return (tmp.tv_sec - base_time.tv_sec) * 1000000 + (tmp.tv_nsec - base_time.tv_nsec) / 1000;
}

//-----------------------------------------------------------------------------
AAssetManager* get_android_asset_manager()
{
	return asset_manager;
}

//-----------------------------------------------------------------------------
bool create_render_window(uint32_t x, uint32_t y, uint32_t width, uint32_t height, bool fullscreen)
{
	return true;
}

//-----------------------------------------------------------------------------
bool destroy_render_window()
{
	return true;
}

//-----------------------------------------------------------------------------
void get_render_window_metrics(uint32_t& width, uint32_t& height)
{
	width = window_width;
	height = window_height;
}

//-----------------------------------------------------------------------------
void swap_buffers()
{
	// not necessary
}

//-----------------------------------------------------------------------------
JNIEXPORT void JNICALL Java_crown_android_CrownLib_initAssetManager(JNIEnv* env, jobject obj, jobject assetManager)
{
	asset_manager = AAssetManager_fromJava(env, assetManager);
}

//-----------------------------------------------------------------------------
JNIEXPORT void JNICALL Java_crown_android_CrownLib_setRenderWindowMetrics(JNIEnv* env, jobject obj, jint width, jint height)
{
	window_width = width;
	window_height = height;
}

} // namespace os
} // namespace crown

