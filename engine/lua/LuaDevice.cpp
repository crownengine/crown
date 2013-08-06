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

#include "Device.h"
#include "LuaEnvironment.h"
#include "LuaStack.h"

namespace crown
{

//-----------------------------------------------------------------------------
CE_EXPORT int device_frame_count(lua_State* L)
{
	LuaStack stack(L);

	uint64_t frame = device()->frame_count();

	stack.push_uint64(frame);

	return 1;
}

//-----------------------------------------------------------------------------
CE_EXPORT int device_last_delta_time(lua_State* L)
{
	LuaStack stack(L);

	float delta = device()->last_delta_time();

	stack.push_float(delta);

	return 1;
}

//-----------------------------------------------------------------------------
CE_EXPORT int device_start(lua_State* /*L*/)
{
	device()->start();

	return 0;
}

//-----------------------------------------------------------------------------
CE_EXPORT int device_stop(lua_State* /*L*/)
{
	device()->stop();

	return 0;
}

//-----------------------------------------------------------------------------
void load_device(LuaEnvironment& env)
{
	env.load_module_function("Device", "frame_count", device_frame_count);
	env.load_module_function("Device", "last_delta_time", device_last_delta_time);
	env.load_module_function("Device", "start", device_start);
	env.load_module_function("Device", "stop", device_stop);
}

} // namespace crown
