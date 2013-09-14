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

#include <windows.h>
#include <process.h>
#include <WinBase.h>

#include "Types.h"
#include "OS.h"

namespace crown
{
namespace os
{

typedef void* (*ThreadFunction)(void*);

class Thread
{
public:

					Thread(const char* name);
					~Thread();

	const char*		name() const;

	void			join();
	void			detach();

	bool			is_running() const;
	bool			is_terminating() const;

	void			start();
	void			stop();

	virtual int32_t	run();

private:

	static void*	background_proc(void* thiz);

private:
	
	const char*		m_name;
	bool			m_is_running;
	bool			m_is_terminating;
	HANDLE			m_thread;
};

} // namespace os
} // namespace crown
