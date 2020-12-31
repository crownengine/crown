/*
 * Copyright (c) 2012-2021 Daniele Bartolini et al.
 * License: https://github.com/dbartolini/crown/blob/master/LICENSE
 */

#include "core/containers/array.inl"
#include "core/math/vector3.inl"
#include "core/memory/globals.h"
#include "core/thread/scoped_mutex.inl"
#include "core/time.h"
#include "device/profiler.h"
#include <new>

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

	const char* buffer_begin()
	{
		return array::begin(*_buffer);
	}

	const char* buffer_end()
	{
		return array::end(*_buffer);
	}

} // namespace profiler_globals

namespace profiler
{
	enum { THREAD_BUFFER_SIZE = 4 * 1024 };
	static char _thread_buffer[THREAD_BUFFER_SIZE];
	static u32 _thread_buffer_size = 0;
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
		if (_thread_buffer_size + 2*sizeof(u32) + sizeof(ev) >= THREAD_BUFFER_SIZE)
			flush_local_buffer();

		char* p = _thread_buffer + _thread_buffer_size;
		*(u32*)p = type;
		p += sizeof(u32);
		*(u32*)p = sizeof(ev);
		p += sizeof(u32);
		*(T*)p = ev;

		_thread_buffer_size += 2*sizeof(u32) + sizeof(ev);
	}

	void enter_profile_scope(const char* name)
	{
		EnterProfileScope ev;
		ev.name = name;
		ev.time = time::now();

		push(ProfilerEventType::ENTER_PROFILE_SCOPE, ev);
	}

	void leave_profile_scope()
	{
		LeaveProfileScope ev;
		ev.time = time::now();

		push(ProfilerEventType::LEAVE_PROFILE_SCOPE, ev);
	}

	void record_float(const char* name, f32 value)
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

	void allocate_memory(const char* name, u32 size)
	{
		AllocateMemory ev;
		ev.name = name;
		ev.size = size;

		push(ProfilerEventType::ALLOCATE_MEMORY, ev);
	}

	void deallocate_memory(const char* name, u32 size)
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
		u32 end = ProfilerEventType::COUNT;
		array::push(*_buffer, (const char*)&end, (u32)sizeof(end));
	}

	void clear()
	{
		array::clear(*_buffer);
	}

} // namespace profiler_globals

} // namespace crown
