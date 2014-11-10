/*
 * Copyright (c) 2012-2014 Daniele Bartolini and individual contributors.
 * License: https://github.com/taylor001/crown/blob/master/LICENSE
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
	typedef Array<char> Buffer;
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
		if (_thread_buffer_size + 2*sizeof(uint32_t) + sizeof(EnterProfileScope) >= THREAD_BUFFER_SIZE)
			flush_local_buffer();

		char* p = _thread_buffer + _thread_buffer_size;
		*(uint32_t*) p = EventType::ENTER_PROFILE_SCOPE;
		(*(uint32_t*)(p + sizeof(uint32_t))) = sizeof(EnterProfileScope);
		(*(EnterProfileScope*)(p + 2*sizeof(uint32_t))).name = name;
		(*(EnterProfileScope*)(p + 2*sizeof(uint32_t))).time = os::clocktime();
		_thread_buffer_size += sizeof(uint32_t) + sizeof(EnterProfileScope);
	}

	void leave_profile_scope()
	{
		if (_thread_buffer_size + 2*sizeof(uint32_t) + sizeof(LeaveProfileScope) >= THREAD_BUFFER_SIZE)
			flush_local_buffer();

		char* p = _thread_buffer + _thread_buffer_size;
		*(uint32_t*) p = EventType::LEAVE_PROFILE_SCOPE;
		(*(uint32_t*)(p + sizeof(uint32_t))) = sizeof(LeaveProfileScope);
		(*(LeaveProfileScope*)(p + 2*sizeof(uint32_t))).time = os::clocktime();
		_thread_buffer_size += sizeof(uint32_t) + sizeof(LeaveProfileScope);
	}

	void record_float(const char* name, float value)
	{
		if (_thread_buffer_size + 2*sizeof(uint32_t) + sizeof(RecordFloat) >= THREAD_BUFFER_SIZE)
			flush_local_buffer();

		char* p = _thread_buffer + _thread_buffer_size;
		*(uint32_t*) p = EventType::RECORD_FLOAT;
		(*(uint32_t*)(p + sizeof(uint32_t))) = sizeof(RecordFloat);
		(*(RecordFloat*)(p + 2*sizeof(uint32_t))).name = name;
		(*(RecordFloat*)(p + 2*sizeof(uint32_t))).value = value;
		_thread_buffer_size += sizeof(uint32_t) + sizeof(RecordFloat);
	}

	void record_vector3(const char* name, const Vector3& value)
	{
		if (_thread_buffer_size + 2*sizeof(uint32_t) + sizeof(RecordVector3) >= THREAD_BUFFER_SIZE)
			flush_local_buffer();

		char* p = _thread_buffer + _thread_buffer_size;
		*(uint32_t*) p = EventType::RECORD_VECTOR3;
		(*(uint32_t*)(p + sizeof(uint32_t))) = sizeof(RecordVector3);
		(*(RecordVector3*)(p + 2*sizeof(uint32_t))).name = name;
		(*(RecordVector3*)(p + 2*sizeof(uint32_t))).value = value;
		_thread_buffer_size += sizeof(uint32_t) + sizeof(RecordVector3);
	}

	void allocate_memory(const char* name, uint32_t size)
	{
		if (_thread_buffer_size + 2*sizeof(uint32_t) + sizeof(AllocateMemory) >= THREAD_BUFFER_SIZE)
			flush_local_buffer();

		char* p = _thread_buffer + _thread_buffer_size;
		*(uint32_t*) p = EventType::ALLOCATE_MEMORY;
		(*(uint32_t*)(p + sizeof(uint32_t))) = sizeof(AllocateMemory);
		(*(AllocateMemory*)(p + 2*sizeof(uint32_t))).name = name;
		(*(AllocateMemory*)(p + 2*sizeof(uint32_t))).size = size;
		_thread_buffer_size += sizeof(uint32_t) + sizeof(AllocateMemory);
	}

	void deallocate_memory(const char* name, uint32_t size)
	{
		if (_thread_buffer_size + 2*sizeof(uint32_t) + sizeof(DeallocateMemory) >= THREAD_BUFFER_SIZE)
			flush_local_buffer();

		char* p = _thread_buffer + _thread_buffer_size;
		*(uint32_t*) p = EventType::DEALLOCATE_MEMORY;
		(*(uint32_t*)(p + sizeof(uint32_t))) = sizeof(DeallocateMemory);
		(*(DeallocateMemory*)(p + 2*sizeof(uint32_t))).name = name;
		(*(DeallocateMemory*)(p + 2*sizeof(uint32_t))).size = size;
		_thread_buffer_size += sizeof(uint32_t) + sizeof(DeallocateMemory);
	}
} // namespace profiler

namespace profiler_globals
{
	void flush()
	{
		profiler::flush_local_buffer();
		uint32_t end = profiler::EventType::COUNT;
		array::push(*_buffer, (const char*)&end, (uint32_t)sizeof(end));
	}

	void clear()
	{
		array::clear(*_buffer);
	}
}
} // namespace crown
