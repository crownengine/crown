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

#include "EventBuffer.h"
#include "Types.h"
#include <cstring>
#include <stdio.h>

namespace crown
{

EventBuffer::EventBuffer() : mBufferCount(0)
{
}

EventBuffer::~EventBuffer()
{
}

void EventBuffer::PushEvent(uint32_t eventType, void* eventData, size_t eventSize)
{
	if (mBufferCount + sizeof(eventType) + eventSize > MAX_EVENT_QUEUE_SIZE)
	{
		Flush();
	}

	uint8_t* q = mBuffer + mBufferCount;

	*(uint32_t*) q = eventType;
	*(size_t*) (q + sizeof(eventType)) = eventSize;
	memcpy(q + sizeof(eventType) + sizeof(eventSize), eventData, eventSize);

	mBufferCount += sizeof(eventType) + sizeof(eventSize) + eventSize;
}

void EventBuffer::PushEventBuffer(uint8_t* eventBuffer, size_t bufferSize)
{
	if (mBufferCount + bufferSize > MAX_EVENT_QUEUE_SIZE)
	{
		Flush();
	}

	uint8_t* q = mBuffer + mBufferCount;
	memcpy(q, eventBuffer, bufferSize);

	mBufferCount += bufferSize;
}

void* EventBuffer::NextEvent(uint32_t& eventType, size_t& size)
{
	static size_t read = 0;

	if (read < mBufferCount)
	{
		uint8_t* q = mBuffer + read;

		eventType = *(uint32_t*)q;
		size = *(size_t*)(q + sizeof(uint32_t));

		read += sizeof(uint32_t) + sizeof(size_t) + size;

		return q + sizeof(uint32_t) + sizeof(size_t);
	}

	read = 0;

	return NULL;
}

void EventBuffer::Clear()
{
	mBufferCount = 0;
}

void EventBuffer::Flush()
{
	mBufferCount = 0;
}

size_t EventBuffer::GetSize() const
{
	return mBufferCount;
}

uint8_t* EventBuffer::GetBuffer()
{
	return mBuffer;
}

} // namespace crown

