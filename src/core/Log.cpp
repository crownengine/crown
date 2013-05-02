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

namespace crown
{

LogLevel Log::m_threshold = LL_DEBUG;
int32_t Log::m_indent_count = 0;

//-----------------------------------------------------------------------------
LogLevel Log::threshold()
{
	return m_threshold;
}

//-----------------------------------------------------------------------------
void Log::set_threshold(LogLevel threshold)
{
	m_threshold = threshold;
}

//-----------------------------------------------------------------------------
void Log::log_message(LogLevel level, const char* message, ::va_list arg)
{
	if (level > m_threshold)
	{
		return;
	}

	switch (level)
	{
		case LL_DEBUG:
			os::log_info(message, arg);
			break;
		case LL_ERROR:
			os::log_error(message, arg);
			break;
		case LL_WARN:
			os::log_warning(message, arg);
			break;
		case LL_INFO:
			os::log_info(message, arg);
			break;
		default:
			break;
	}
}

//-----------------------------------------------------------------------------
void Log::d(const char* message, ...)
{
	va_list args;
	va_start (args, message);
	log_message(LL_DEBUG, message, args);
	va_end (args);
}

//-----------------------------------------------------------------------------
void Log::e(const char* message, ...)
{
	va_list args;
	va_start (args, message);
	log_message(LL_ERROR, message, args);
	va_end (args);
}

//-----------------------------------------------------------------------------
void Log::w(const char* message, ...)
{
	va_list args;
	va_start (args, message);
	log_message(LL_WARN, message, args);
	va_end (args);
}

//-----------------------------------------------------------------------------
void Log::i(const char* message, ...)
{
	va_list args;
	va_start (args, message);
	log_message(LL_INFO, message, args);
	va_end (args);
}

//-----------------------------------------------------------------------------
void Log::indent_in()
{
	m_indent_count += 1;
}

//-----------------------------------------------------------------------------
void Log::indent_out()
{
	if (m_indent_count > 0)
	{
		m_indent_count -= 1;
	}
}

} // namespace crown

