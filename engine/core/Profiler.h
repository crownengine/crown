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

#include "OS.h"
#include "Memory.h"
#include "Mutex.h"
#include "ScopedMutex.h"
#include "Macros.h"

namespace crown
{
namespace profiler
{
	const char* const GLOBAL_FLOAT = "global.float";

	struct EventType
	{
		enum Enum
		{
			ENTER_PROFILE_SCOPE = 0,
			LEAVE_PROFILE_SCOPE = 1,
			RECORD_FLOAT = 2,
			RECORD_VECTOR3 = 3
		};
	};

	struct RecordFloat
	{
		const char* name;
		float value; 
	};

	struct RecordVector3
	{
		const char* name;
		Vector3 value;
	};

	struct EnterProfileScope
	{
		const char* name;
		double time;
	};

	struct LeaveProfileScope
	{
		double time;
	};

	static Array<char> g_buffer(default_allocator());
	static Mutex g_buffer_mutex;

	#define THREAD_BUFFER_SIZE 1024
	CE_THREAD char t_buffer[THREAD_BUFFER_SIZE];
	CE_THREAD uint32_t t_buffer_size = 0;

	inline void flush_local_buffer()
	{
		ScopedMutex sm(g_buffer_mutex);
		//cg_buffer.push(t_buffer, t_buffer_size);

		uint32_t size = t_buffer_size;
		t_buffer_size = 0;

		uint32_t cur = 0;

		// TODO TODO TODO TODO TODO TODO

		// while (cur < size)
		// {
		// 	char* p = t_buffer + cur;
		// 	uint32_t event_type = *(uint32_t*) p;
		// 	RecordFloat event = *(RecordFloat*)(p + sizeof(uint32_t));
		// 	Log::d("ev type = %d, name = %s, value = %f\n", event_type, event.name, 1.0 / event.value);
		// 	cur += sizeof(uint32_t) + sizeof(RecordFloat);
		// }
	}

	inline void enter_profile_scope(const char* name)
	{
		if (t_buffer_size + sizeof(uint32_t) + sizeof(EnterProfileScope) >= THREAD_BUFFER_SIZE)
		{
			flush_local_buffer();
		}

		char* p = t_buffer + t_buffer_size;
		*(uint32_t*) p = EventType::ENTER_PROFILE_SCOPE;
		(*(EnterProfileScope*)(p + sizeof(uint32_t))).name = name;
		(*(EnterProfileScope*)(p + sizeof(uint32_t))).time = os::milliseconds();
		t_buffer_size += sizeof(uint32_t) + sizeof(EnterProfileScope);
	}

	inline void leave_profile_scope()
	{
		if (t_buffer_size + sizeof(uint32_t) + sizeof(LeaveProfileScope) >= THREAD_BUFFER_SIZE)
		{
			flush_local_buffer();
		}

		char* p = t_buffer + t_buffer_size;
		*(uint32_t*) p = EventType::LEAVE_PROFILE_SCOPE;
		(*(LeaveProfileScope*)(p + sizeof(uint32_t))).time = os::milliseconds();
		t_buffer_size += sizeof(uint32_t) + sizeof(LeaveProfileScope);
	}

	inline void record_float(const char* name, float value)
	{
		if (t_buffer_size + sizeof(uint32_t) + sizeof(RecordFloat) >= THREAD_BUFFER_SIZE)
		{
			flush_local_buffer();
		}

		char* p = t_buffer + t_buffer_size;
		*(uint32_t*) p = EventType::RECORD_FLOAT;
		(*(RecordFloat*)(p + sizeof(uint32_t))).name = name;
		(*(RecordFloat*)(p + sizeof(uint32_t))).value = value;
		t_buffer_size += sizeof(uint32_t) + sizeof(RecordFloat);
	}

	inline void record_vector3(const char* name, const Vector3& value)
	{
		if (t_buffer_size + sizeof(uint32_t) + sizeof(RecordVector3) >= THREAD_BUFFER_SIZE)
		{
			flush_local_buffer();
		}

		char* p = t_buffer + t_buffer_size;
		*(uint32_t*) p = EventType::RECORD_VECTOR3;
		(*(RecordVector3*)(p + sizeof(uint32_t))).name = name;
		(*(RecordVector3*)(p + sizeof(uint32_t))).value = value;
		t_buffer_size += sizeof(uint32_t) + sizeof(RecordVector3);
	}
}

} // namespace crown
