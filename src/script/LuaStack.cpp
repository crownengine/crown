#include "LuaStack.h"

namespace crown
{

//-------------------------------------------------------	
LuaStack::LuaStack(lua_State* L)
{
	m_state = L;
}

//-------------------------------------------------------
void LuaStack::push_bool(bool value)
{
	lua_pushboolean(m_state, value);
}

//-------------------------------------------------------
void LuaStack::push_int(int32_t value)
{
	lua_pushinteger(m_state, value);
}

//-------------------------------------------------------
void LuaStack::push_float(float value)
{
	lua_pushnumber(m_state, value);
}

//-------------------------------------------------------
void LuaStack::push_string(const char* str, size_t len)
{
	lua_pushlstring(m_state, str, len);
}

//-------------------------------------------------------
void LuaStack::push_lightudata(void* ptr)
{
	lua_pushlightuserdata(m_state, ptr);
}

//-------------------------------------------------------
bool LuaStack::get_bool(int32_t index)
{
	return (bool) luaL_checkinteger(m_state, index);
}

//-------------------------------------------------------
int32_t LuaStack::get_int(int32_t index)
{
	return luaL_checkinteger(m_state, index);
}

//-------------------------------------------------------
float LuaStack::get_float(int32_t index)
{
	return luaL_checknumber(m_state, index);
}

//-------------------------------------------------------
const char* LuaStack::get_string(int32_t index)
{
	return luaL_checkstring(m_state, index);
}

} // namespace crown