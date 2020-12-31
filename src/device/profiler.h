/*
 * Copyright (c) 2012-2021 Daniele Bartolini et al.
 * License: https://github.com/dbartolini/crown/blob/master/LICENSE
 */

#pragma once

#include "core/math/types.h"
#include "core/types.h"

namespace crown
{
/// Enumerates profiler event types.
///
/// @ingroup Device
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
	f32 value;
};

struct RecordVector3
{
	const char* name;
	Vector3 value;
};

struct EnterProfileScope
{
	const char* name;
	s64 time;
};

struct LeaveProfileScope
{
	s64 time;
};

struct AllocateMemory
{
	const char* name;
	u32 size;
};

struct DeallocateMemory
{
	const char* name;
	u32 size;
};

/// Functions to access profiler.
///
/// @ingroup Device
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

	/// Records the f32 @a value with the given @a name.
	void record_float(const char* name, f32 value);

	/// Records the vector3 @a value with the given @a name.
	void record_vector3(const char* name, const Vector3& value);

	/// Records a memory allocation of @a size with the given @a name.
	void allocate_memory(const char* name, u32 size);

	/// Records a memory deallocation of @a size with the given @a name.
	void deallocate_memory(const char* name, u32 size);

} // namespace profiler

namespace profiler_globals
{
	void init();
	void shutdown();

	const char* buffer_begin();
	const char* buffer_end();
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
	#define ENTER_PROFILE_SCOPE(name) CE_NOOP()
	#define LEAVE_PROFILE_SCOPE() CE_NOOP()
	#define RECORD_FLOAT(name, value) CE_NOOP()
	#define RECORD_VECTOR3(name, value) CE_NOOP()
	#define ALLOCATE_MEMORY(name, size) CE_NOOP()
	#define DEALLOCATE_MEMORY(name, size) CE_NOOP()
#endif // CROWN_DEBUG
