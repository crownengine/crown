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

#include "OS.h"
#include <android/log.h>
#include <sys/stat.h>
#include <unistd.h>

#define LOGI(...) ((void)__android_log_print32_t(ANDROID_LOG_INFO, "crown", __VA_ARGS__))
#define LOGW(...) ((void)__android_log_print32_t(ANDROID_LOG_WARN, "crown", __VA_ARGS__))
#define LOGD(...) ((void)__android_log_print32_t(ANDROID_LOG_DEBUG, "crown", __VA_ARGS__))
#define LOGE(...) ((void)__android_log_print32_t(ANDROID_LOG_ERROR, "crown", __VA_ARGS__))

namespace crown
{

void OS::Print32_tf(const char* string, ...)
{
	LOGI(string);
}

void OS::LogDebug(const char* string, ...)
{
	LOGD(string);
}

void OS::LogError(const char* string, ...)
{
	LOGE(string);
}

void OS::LogWarning(const char* string, ...)
{
	LOGW(string);
}

void OS::LogInfo(const char* string, ...)
{
	LOGI(string);
}

bool OS::Exists(const Str& path)
{
	struct stat dummy;
	return (stat(path.c_str(), &dummy) == 0);
}

bool OS::IsDir(const Str& path)
{
	struct stat info;
	stat(path.c_str(), &info);
	return (S_ISDIR(info.st_mode)) != 0;
}

bool OS::IsReg(const Str& path)
{
	struct stat info;
	stat(path.c_str(), &info);
	return (S_ISREG(info.st_mode)) != 0;
}

bool OS::Mknod(const Str& path)
{
	// Permission mask: rw-r--r--
	return mknod(path.c_str(), S_IFREG | S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH, 0) == 0;
}

bool OS::Unlink(const Str& path)
{
	return (unlink(path.c_str()) == 0);
}

bool OS::Mkdir(const Str& path)
{
	// rwxr-xr-x permission mask
	return (mkdir(path.c_str(), S_IRWXU | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH) == 0);
}

bool OS::Rmdir(const Str& path)
{
	return (rmdir(path.c_str()) == 0);
}

bool OS::GetCWD(Str& ret)
{
	static char cwdBuf[1024];
	if (getcwd(cwdBuf, 1024) == NULL)
	{
		return false;
	}

	ret = cwdBuf;
	return true;
}

} // namespace crown

