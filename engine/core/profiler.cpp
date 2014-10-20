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

#include "profiler.h"
#include "os.h"
#include "array.h"
#include "mutex.h"
#include "memory.h"

namespace crown
{
namespace profiler_globals
{
	char _mem[sizeof(Buffer)];
	Buffer* _buffer = NULL;

	void init()
	{
		_buffer = new (_mem)Buffer(default_allocator());
	}

	void shutdown()
	{
		_buffer->~Buffer();
		_buffer = NULL;
	}

	const Buffer& buffer()
	{
		return *_buffer;
	}
} // namespace profiler_globals

namespace profiler
{
	enum { THREAD_BUFFER_SIZE = 4 * 1024 };
	char _thread_buffer[THREAD_BUFFER_SIZE];
	uint32_t _thread_buffer_size = 0;
	Mutex _buffer_mutex;

	void flush_local_buffer()
	{
		ScopedMutex sm(_buffer_mutex);
		array::push(*profiler_globals::_buffer, _thread_buffer, _thread_buffer_size);
		_thread_buffer_size = 0;
	}

	void enter_profile_scope(const char* name)
	{
		if (_thread_buffer_size + sizeof(uint32_t) + sizeof(EnterProfileScope) >= THREAD_BUFFER_SIZE)
			flush_local_buffer();

		char* p = _thread_buffer + _thread_buffer_size;
		*(uint32_t*) p = EventType::ENTER_PROFILE_SCOPE;
		(*(EnterProfileScope*)(p + sizeof(uint32_t))).name = name;
		(*(EnterProfileScope*)(p + sizeof(uint32_t))).time = os::clocktime();
		_thread_buffer_size += sizeof(uint32_t) + sizeof(EnterProfileScope);
	}

	void leave_profile_scope()
	{
		if (_thread_buffer_size + sizeof(uint32_t) + sizeof(LeaveProfileScope) >= THREAD_BUFFER_SIZE)
			flush_local_buffer();

		char* p = _thread_buffer + _thread_buffer_size;
		*(uint32_t*) p = EventType::LEAVE_PROFILE_SCOPE;
		(*(LeaveProfileScope*)(p + sizeof(uint32_t))).time = os::clocktime();
		_thread_buffer_size += sizeof(uint32_t) + sizeof(LeaveProfileScope);
	}

	void record_float(const char* name, float value)
	{
		if (_thread_buffer_size + sizeof(uint32_t) + sizeof(RecordFloat) >= THREAD_BUFFER_SIZE)
			flush_local_buffer();

		char* p = _thread_buffer + _thread_buffer_size;
		*(uint32_t*) p = EventType::RECORD_FLOAT;
		(*(RecordFloat*)(p + sizeof(uint32_t))).name = name;
		(*(RecordFloat*)(p + sizeof(uint32_t))).value = value;
		_thread_buffer_size += sizeof(uint32_t) + sizeof(RecordFloat);
	}

	void record_vector3(const char* name, const Vector3& value)
	{
		if (_thread_buffer_size + sizeof(uint32_t) + sizeof(RecordVector3) >= THREAD_BUFFER_SIZE)
			flush_local_buffer();

		char* p = _thread_buffer + _thread_buffer_size;
		*(uint32_t*) p = EventType::RECORD_VECTOR3;
		(*(RecordVector3*)(p + sizeof(uint32_t))).name = name;
		(*(RecordVector3*)(p + sizeof(uint32_t))).value = value;
		_thread_buffer_size += sizeof(uint32_t) + sizeof(RecordVector3);
	}
} // namespace profiler

namespace profiler_globals
{
	void flush()
	{
		profiler::flush_local_buffer();	
	}

	void clear()
	{
		array::clear(*_buffer);
	}
}
} // namespace crown
