/*
 * Copyright (c) 2012-2016 Daniele Bartolini and individual contributors.
 * License: https://github.com/taylor001/crown/blob/master/LICENSE
 */

#include "profiler.h"
#include "os.h"
#include "array.h"
#include "mutex.h"
#include "memory.h"
#include "vector3.h"

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

	const char* buffer()
	{
		return array::begin(*_buffer);
	}
} // namespace profiler_globals

namespace profiler
{
	enum { THREAD_BUFFER_SIZE = 4 * 1024 };
	static char _thread_buffer[THREAD_BUFFER_SIZE];
	static uint32_t _thread_buffer_size = 0;
	static Mutex _buffer_mutex;

	static void flush_local_buffer()
	{
		ScopedMutex sm(_buffer_mutex);
		array::push(*profiler_globals::_buffer, _thread_buffer, _thread_buffer_size);
		_thread_buffer_size = 0;
	}

	template <typename T>
	static void push(ProfilerEventType::Enum type, const T& ev)
	{
		if (_thread_buffer_size + 2*sizeof(uint32_t) + sizeof(ev) >= THREAD_BUFFER_SIZE)
			flush_local_buffer();

		char* p = _thread_buffer + _thread_buffer_size;
		*(uint32_t*)p = type;
		p += sizeof(uint32_t);
		*(uint32_t*)p = sizeof(ev);
		p += sizeof(uint32_t);
		*(T*)p = ev;

		_thread_buffer_size += 2*sizeof(uint32_t) + sizeof(ev);
	}

	void enter_profile_scope(const char* name)
	{
		EnterProfileScope ev;
		ev.name = name;
		ev.time = os::clocktime();

		push(ProfilerEventType::ENTER_PROFILE_SCOPE, ev);
	}

	void leave_profile_scope()
	{
		LeaveProfileScope ev;
		ev.time = os::clocktime();

		push(ProfilerEventType::LEAVE_PROFILE_SCOPE, ev);
	}

	void record_float(const char* name, float value)
	{
		RecordFloat ev;
		ev.name = name;
		ev.value = value;

		push(ProfilerEventType::RECORD_FLOAT, ev);
	}

	void record_vector3(const char* name, const Vector3& value)
	{
		RecordVector3 ev;
		ev.name = name;
		ev.value = value;

		push(ProfilerEventType::RECORD_VECTOR3, ev);
	}

	void allocate_memory(const char* name, uint32_t size)
	{
		AllocateMemory ev;
		ev.name = name;
		ev.size = size;

		push(ProfilerEventType::ALLOCATE_MEMORY, ev);
	}

	void deallocate_memory(const char* name, uint32_t size)
	{
		DeallocateMemory ev;
		ev.name = name;
		ev.size = size;

		push(ProfilerEventType::DEALLOCATE_MEMORY, ev);
	}
} // namespace profiler

namespace profiler_globals
{
	void flush()
	{
		profiler::flush_local_buffer();
		uint32_t end = ProfilerEventType::COUNT;
		array::push(*_buffer, (const char*)&end, (uint32_t)sizeof(end));
	}

	void clear()
	{
		array::clear(*_buffer);
	}
}
} // namespace crown
