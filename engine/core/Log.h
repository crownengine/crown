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

#include <cstdio>
#include <cstdarg>
#include "Types.h"

namespace crown
{

/// Enumerates log levels.
struct LogSeverity
{
	enum Enum
	{
		INFO	= 0,
		WARN	= 1,
		ERROR	= 2,
		DEBUG	= 3
	};	
};

class RPCServer;

/// Used to log messages.
class CE_EXPORT Log
{

public:

	/// Returns the threshold used to filter out log messages.
	static LogSeverity::Enum threshold();

	/// Sets the thresold used to filter out log messages
	static void			set_threshold(LogSeverity::Enum threshold);

	static void			log_message(LogSeverity::Enum severity, const char* message, ::va_list arg);

	static void			d(const char* message, ...);
	static void			e(const char* message, ...);
	static void			w(const char* message, ...);
	static void			i(const char* message, ...);
	static void			d1(const char* message, ::va_list args);
	static void			e1(const char* message, ::va_list args);
	static void			w1(const char* message, ::va_list args);
	static void			i1(const char* message, ::va_list args);

private:


	static LogSeverity::Enum m_threshold;
};

} // namespace crown

