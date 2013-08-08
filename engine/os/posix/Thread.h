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

#include <pthread.h>

#include "Types.h"

namespace crown
{

typedef void* (*ThreadFunction)(void*);

class Thread
{
public:

	/// Constructs the thread and gives it a @a name.
	/// @note
	/// The actual OS thread creation and execution is
	/// deferred to the first call to Thread::start().
					Thread(const char* name);

	/// Does not stop the thread. The user must call
	/// Thread::stop() to effectively stop the thread. 
	virtual			~Thread();

	/// Returns the name of the thread.
	const char*		name() const;

	void			join();
	void			detach();

	/// Returns whether the thread is currently running.
	bool			is_running() const;

	/// Returns whether the thread is being asked to stop running.
	/// @note
	/// The implementer tipically polls this function to
	/// determine whether to stop the execution or not.
	bool			is_terminating() const;

	/// Starts the execution of the thread.
	/// The function creates the OS thread and starts
	/// its execution.
	void			start();

	/// Stops the execution of the thread if it is running.
	/// The function releases the OS thread causing its
	/// termination.
	void			stop();

	/// Executes in background when the thead is running.
	/// The thread has to be started with Thread::start()
	virtual int32_t	run();

private:

	static void*	background_proc(void* thiz);

private:

	const char*		m_name;
	bool			m_is_running;
	bool			m_is_terminating;
	pthread_t		m_thread;

private:

	// Disable copying
					Thread(const Thread&);
	Thread&			operator=(const Thread&);
};

} // namespace crown
