/*
 * Copyright (c) 2012-2014 Daniele Bartolini and individual contributors.
 * License: https://github.com/taylor001/crown/blob/master/LICENSE
 */

#pragma once

#include "math_types.h"
#include "container_types.h"

namespace crown
{
namespace profiler
{
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
		int64_t time;
	};

	struct LeaveProfileScope
	{
		int64_t time;
	};

	void enter_profile_scope(const char* name);
	void leave_profile_scope();
	void record_float(const char* name, float value);
	void record_vector3(const char* name, const Vector3& value);

#ifdef CROWN_DEBUG
	#define ENTER_PROFILE_SCOPE(name) enter_profile_scope(name);
	#define LEAVE_PROFILE_SCOPE() leave_profile_scope(name);
	#define RECORD_FLOAT(name, value) record_float(name, value);
	#define RECORD_VECTOR3(name, value) record_vector3(name, value);
#else
	#define ENTER_PROFILE_SCOPE(name) ((void)0)
	#define LEAVE_PROFILE_SCOPE() ((void)0)
	#define RECORD_FLOAT(name, value) ((void)0)
	#define RECORD_VECTOR3(name, value) ((void)0)
#endif
} // namespace profiler

namespace profiler_globals
{
	void init();
	void shutdown();

	typedef Array<char> Buffer;
	const Buffer& buffer();
	void flush();
	void clear();
} // namespace profiler_globals
} // namespace crown
