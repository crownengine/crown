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

#include "Types.h"

#define MAX_EVENT_QUEUE_SIZE 64 * 1024

namespace crown
{

/**
	Buffer of variable-lenght events.

	[type1][size1][struct1][type2][size2][struct2]...
*/
class EventBuffer
{

public:

				EventBuffer();
				~EventBuffer();

	void		PushEvent(uint eventType, void* eventData, size_t eventSize);
	void		PushEventBuffer(char* eventBuffer, size_t bufferSize);

	void*		NextEvent(uint& eventType, size_t& size);

	void		Clear();
	void		Flush();

	size_t		GetSize() const;
	char*		GetBuffer();

public:

	size_t		mBufferCount;					// Number of bytes in the buffer
	char		mBuffer[MAX_EVENT_QUEUE_SIZE];
};

} // namespace crown

