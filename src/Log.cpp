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

#include "Log.h"
#include "OS.h"
#include <cstdarg>

namespace Crown
{

LogLevel Log::mThreshold = LL_DEBUG;
int Log::mIndentCount = 0;

LogLevel Log::GetThreshold()
{
	return mThreshold;
}

void Log::SetThreshold(LogLevel threshold)
{
	mThreshold = threshold;
}

void Log::LogMessage(LogLevel level, const char* message, va_list arg)
{
	if (level > mThreshold)
	{
		return;
	}

	switch (level)
	{
		case LL_DEBUG:
			OS::LogInfo(message, arg);
			break;
		case LL_ERROR:
			OS::LogError(message, arg);
			break;
		case LL_WARN:
			OS::LogWarning(message, arg);
			break;
		case LL_INFO:
			OS::LogInfo(message, arg);
			break;
		default:
			break;
	}
}

void Log::D(const char* message, ...)
{
	va_list args;
	va_start (args, message);
	LogMessage(LL_DEBUG, message, args);
	va_end (args);
}

void Log::E(const char* message, ...)
{
	va_list args;
	va_start (args, message);
	LogMessage(LL_ERROR, message, args);
	va_end (args);
}

void Log::W(const char* message, ...)
{
	va_list args;
	va_start (args, message);
	LogMessage(LL_WARN, message, args);
	va_end (args);
}

void Log::I(const char* message, ...)
{
	va_list args;
	va_start (args, message);
	LogMessage(LL_INFO, message, args);
	va_end (args);
}

void Log::IndentIn()
{
	mIndentCount += 1;
}

void Log::IndentOut()
{
	if (mIndentCount >= 1)
	{
		mIndentCount -= 1;
	}
}

} // namespace Crown

