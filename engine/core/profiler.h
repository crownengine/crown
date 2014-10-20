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
