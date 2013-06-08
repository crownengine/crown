#include "Device.h"
#include "LuaEnvironment.h"
#include "LuaStack.h"

namespace crown
{

extern "C"
{

//-----------------------------------------------------------------------------
int32_t device_stop(lua_State* L)
{
	device()->stop();

	return 0;
}

} // extern "C"

//-----------------------------------------------------------------------------
void load_device(LuaEnvironment& env)
{
	env.load_module_function("Device", "stop", device_stop);
}

} // namespace crown
