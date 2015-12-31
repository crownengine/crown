/*
 * Copyright (c) 2012-2015 Daniele Bartolini and individual contributors.
 * License: https://github.com/taylor001/crown/blob/master/LICENSE
 */

#pragma once

#include "types.h"
#include "math_types.h"

namespace crown
{

/// @defgroup Profiler Profiler

struct ProfilerEventType
{
	enum Enum
	{
		ENTER_PROFILE_SCOPE,
		LEAVE_PROFILE_SCOPE,
		RECORD_FLOAT,
		RECORD_VECTOR3,
		ALLOCATE_MEMORY,
		DEALLOCATE_MEMORY,

		COUNT
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
	int64_t time;
};

struct LeaveProfileScope
{
	int64_t time;
};

struct AllocateMemory
{
	const char* name;
	uint32_t size;
};

struct DeallocateMemory
{
	const char* name;
	uint32_t size;
};

/// Functions to access profiler.
///
/// @ingroup Profiler
///
/// @note
/// The profiler does not copy pointer data.
/// You have to store it somewhere and make sure it is
/// valid throughout the program execution.
namespace profiler
{
	/// Starts a new profile scope with the given @a name.
	void enter_profile_scope(const char* name);

	/// Ends the last profile scope.
	void leave_profile_scope();

	/// Records the float @a value with the given @a name.
	void record_float(const char* name, float value);

	/// Records the vector3 @a value with the given @a name.
	void record_vector3(const char* name, const Vector3& value);

	/// Records a memory allocation of @a size with the given @a name.
	void allocate_memory(const char* name, uint32_t size);

	/// Records a memory deallocation of @a size with the given @a name.
	void deallocate_memory(const char* name, uint32_t size);
} // namespace profiler

namespace profiler_globals
{
	void init();
	void shutdown();

	const char* buffer();
	void flush();
	void clear();
} // namespace profiler_globals
} // namespace crown

#if CROWN_DEBUG
	#define ENTER_PROFILE_SCOPE(name) profiler::enter_profile_scope(name)
	#define LEAVE_PROFILE_SCOPE() profiler::leave_profile_scope()
	#define RECORD_FLOAT(name, value) profiler::record_float(name, value)
	#define RECORD_VECTOR3(name, value) profiler::record_vector3(name, value)
	#define ALLOCATE_MEMORY(name, size) profiler::allocate_memory(name, size)
	#define DEALLOCATE_MEMORY(name, size) profiler::deallocate_memory(name, size)
#else
	#define ENTER_PROFILE_SCOPE(name) ((void)0)
	#define LEAVE_PROFILE_SCOPE() ((void)0)
	#define RECORD_FLOAT(name, value) ((void)0)
	#define RECORD_VECTOR3(name, value) ((void)0)
	#define ALLOCATE_MEMORY(name, size) ((void)0)
	#define DEALLOCATE_MEMORY(name, size) ((void)0)
#endif // CROWN_DEBUG
