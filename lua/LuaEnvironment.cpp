#include "LuaEnvironment.h"

namespace crown
{

LuaEnvironment::LuaEnvironment(lua_State* L) :
	m_state(L)
{

}

//-----------------------------------------------------------
void LuaEnvironment::load_module_function(const char* module, const char* name, const lua_CFunction func)
{
	luaL_Reg entry[2];

	entry[0].name = name;
	entry[0].func = func;

	entry[1].name = NULL;
	entry[1].func = NULL;

	luaL_register(m_state, module, entry);
}


int32_t luaopen_libcrownlua(lua_State* L)
{
	LuaEnvironment env(L);

	load_vec2(env);
	load_vec3(env);
	load_mat4(env);
	load_quat(env);
	load_math(env);

	load_mouse(env);
	load_keyboard(env);
	load_accelerometer(env);

	return 1;
}


} // namespace crown