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

#include <windows.h>
#include "Timer.h"

namespace crown
{

Timer::Timer()
{
	mPreciseTimer = (QueryPerformanceFrequency((LARGE_INTEGER*) &mFrequency) == TRUE);

	if (mPreciseTimer)
	{
		QueryPerformanceCounter((LARGE_INTEGER*) &mCreationTime);
	}
	else
	{
		mFrequency = 1000;
		mCreationTime = timeGetTime();
	}

	Reset();
}

Timer::~Timer()
{
}

void Timer::Reset()
{
	if (mPreciseTimer)
	{
		QueryPerformanceCounter((LARGE_INTEGER*) &mCreationTime);
	}
	else
	{
		mCreationTime = timeGetTime();
	}
}

uint64_t Timer::GetMilliseconds() const
{
	uint64_t currentTime;

	if (mPreciseTimer)
	{
		QueryPerformanceCounter((LARGE_INTEGER*) &currentTime);
	}
	else
	{
		currentTime = timeGetTime();
	}

	return (uint64_t) ((currentTime - mCreationTime) / (mFrequency / 1000));
}

uint64_t Timer::GetMicroseconds() const
{
	uint64_t currentTime;

	if (mPreciseTimer)
	{
		QueryPerformanceCounter((LARGE_INTEGER*) &currentTime);
	}
	else
	{
		currentTime = timeGetTime();
	}

	//I'm using doubles here because if the timer isn't precise, it's always going to say
	//0us because of the int32_teger calculus.
	return (uint64_t)((currentTime - mCreationTime) / (mFrequency/1000000.0));
}


void Timer::StartMilliseconds()
{
	if (mPreciseTimer)
	{
		QueryPerformanceCounter((LARGE_INTEGER*) &mStartTime);
	}
	else
	{
		mStartTime = timeGetTime();
	}
}

uint64_t Timer::StopMilliseconds() const
{
	uint64_t currentTime;

	if (mPreciseTimer)
	{
		QueryPerformanceCounter((LARGE_INTEGER*) &currentTime);
	}
	else
	{
		currentTime = timeGetTime();
	}

	return (uint64_t) ((currentTime - mStartTime) / (mFrequency / 1000));
}

void Timer::StartMicroseconds()
{
	if (mPreciseTimer)
	{
		QueryPerformanceCounter((LARGE_INTEGER*) &mStartTime);
	}
	else
	{
		mStartTime = timeGetTime();
	}
}

uint64_t Timer::StopMicroseconds() const
{
	uint64_t currentTime;

	if (mPreciseTimer)
	{
		QueryPerformanceCounter((LARGE_INTEGER*) &currentTime);
	}
	else
	{
		currentTime = timeGetTime();
	}

	//I'm using doubles here because if the timer isn't precise, it's always going to say
	//0us because of the int32_teger calculus.
	return (uint64_t)((currentTime - mStartTime) / (mFrequency/1000000.0));
}

} // namespace crown

