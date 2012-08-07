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

#include <sys/time.h>
#include "Types.h"

namespace Crown
{

class Timer
{

public:

	//! Constructor
	Timer();

	//! Destructor
	~Timer();

	//! Returns the time (in milliseconds) elapsed since the instantiation of this class
	ulong GetMilliseconds() const;

	//! Returns the time (in microseconds) elapsed since the instantiation of this class
	ulong GetMicroseconds() const;

	//! Records the current time
	void StartMilliseconds();

	//! Returns the time (in milliseconds) elapsed since the last call to StartMilliseconds()
	ulong StopMilliseconds() const;

	//! Records the current time
	void StartMicroseconds();

	//! Returns the time (in microseconds) elapsed since the last call to StartMicroseconds()
	ulong StopMicroseconds() const;

private:

	// Records the initial reference time
	void Reset();

	timespec mCreationTime;		// Time at instantiation
	timespec mStartTime;		// Time at Start* call
};

} // namespace Crown

