#include "Device.h"
#include "LuaEnvironment.h"
#include "LuaStack.h"

namespace crown
{

//-----------------------------------------------------------------------------
CE_EXPORT int32_t device_frame_count(lua_State* L)
{
	LuaStack stack(L);

	uint64_t frame = device()->frame_count();

	stack.push_uint64(frame);

	return 1;
}

//-----------------------------------------------------------------------------
CE_EXPORT int32_t device_last_delta_time(lua_State* L)
{
	LuaStack stack(L);

	float delta = device()->last_delta_time();

	stack.push_float(delta);

	return 1;
}

//-----------------------------------------------------------------------------
CE_EXPORT int32_t device_start(lua_State* L)
{
	device()->start();

	return 0;
}

//-----------------------------------------------------------------------------
CE_EXPORT int32_t device_stop(lua_State* L)
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
