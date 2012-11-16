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

namespace crown
{

/**
	Enumerates log levels.
*/
enum LogLevel
{
	LL_INFO		= 0,
	LL_WARN		= 1,
	LL_ERROR	= 2,
	LL_DEBUG	= 3
};

class Log
{

public:

	static LogLevel		GetThreshold();
	static void			SetThreshold(LogLevel threshold);

	static void			LogMessage(LogLevel level, const char* message, va_list arg);

	static void			D(const char* message, ...);
	static void			E(const char* message, ...);
	static void			W(const char* message, ...);
	static void			I(const char* message, ...);

	static void			IndentIn();
	static void			IndentOut();

private:

	static LogLevel		mThreshold;
	static int32_t			mIndentCount;
};

} // namespace crown

