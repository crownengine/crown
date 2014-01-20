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

#include "Log.h"
#include "Device.h"
#include "ConsoleServer.h"

namespace crown
{

LogSeverity::Enum Log::m_threshold = LogSeverity::DEBUG;

//-----------------------------------------------------------------------------
LogSeverity::Enum Log::threshold()
{
	return m_threshold;
}

//-----------------------------------------------------------------------------
void Log::set_threshold(LogSeverity::Enum threshold)
{
	m_threshold = threshold;
}

//-----------------------------------------------------------------------------
void Log::log_message(LogSeverity::Enum severity, const char* message, ::va_list arg)
{
	if (severity > m_threshold)
	{
		return;
	}

	va_list arg_copy;
	__va_copy(arg_copy, arg);

	char buf[1024];
	int len = vsnprintf(buf, 1024 - 2, message, arg);
	buf[len] = '\n';
	buf[len + 1] = '\0';

	// Log on local device
	switch (severity)
	{
		case LogSeverity::DEBUG: os::log_debug(message, arg_copy); break;
		case LogSeverity::WARN: os::log_warning(message, arg_copy); break;
		case LogSeverity::ERROR: os::log_error(message, arg_copy); break;
		case LogSeverity::INFO: os::log_info(message, arg_copy); break;
		default: break;
	}
	va_end(arg_copy);

	// Log to remote clients
	if (device()->console() != NULL && string::strlen(buf) > 0)
	{
		device()->console()->log_to_all(buf, severity);
	}
}

//-----------------------------------------------------------------------------
void Log::d(const char* message, ...)
{
	va_list args;
	va_start (args, message);
	log_message(LogSeverity::DEBUG, message, args);
	va_end (args);
}

//-----------------------------------------------------------------------------
void Log::e(const char* message, ...)
{
	va_list args;
	va_start (args, message);
	log_message(LogSeverity::ERROR, message, args);
	va_end (args);
}

//-----------------------------------------------------------------------------
void Log::w(const char* message, ...)
{
	va_list args;
	va_start (args, message);
	log_message(LogSeverity::WARN, message, args);
	va_end (args);
}

//-----------------------------------------------------------------------------
void Log::i(const char* message, ...)
{
	va_list args;
	va_start (args, message);
	log_message(LogSeverity::INFO, message, args);
	va_end (args);
}

} // namespace crown
