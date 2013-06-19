#include "LuaStack.h"
#include "Device.h"
#include "LuaEnvironment.h"
#include "Accelerometer.h"

namespace crown
{

//-----------------------------------------------------------------------------
CE_EXPORT int32_t accelerometer_orientation(lua_State* L)
{
	LuaStack stack(L);

	Vec3* orientation = next_vec3();
	*orientation = device()->accelerometer()->orientation();

	stack.push_vec3(orientation);

	return 1;
}

//-----------------------------------------------------------------------------
void load_accelerometer(LuaEnvironment& env)
{
	env.load_module_function("Accelerometer", "orientation", accelerometer_orientation);
}

} // namespace crown
