#include "Device.h"
#include "LuaEnvironment.h"
#include "LuaStack.h"

namespace crown
{

//-----------------------------------------------------------------------------
CE_EXPORT int32_t device_stop(lua_State* L)
{
	device()->stop();

	return 0;
}

//-----------------------------------------------------------------------------
void load_device(LuaEnvironment& env)
{
	env.load_module_function("Device", "stop", device_stop);
}

} // namespace crown
