#pragma once

#include "lua.hpp"
#include <iostream>

namespace crown
{
class LuaState
{
public:

		LuaState()
		{
			m_state = luaL_newstate();
			luaL_openlibs(m_state);
		}

		~LuaState()
		{
			lua_close(m_state);
		}

		lua_State* state()
		{
			return m_state;
		}

		int load_program(const char* src)
		{
			return luaL_loadfile(m_state, src);
		}

private:

	lua_State* m_state;

};

static int execute_program(lua_State* state)
{
	return lua_pcall(state, 0, LUA_MULTRET, 0);
}

static void report_errors(lua_State* state, const int status)
{
	if (status != 0)
	{
		std::cerr << "-- " << lua_tostring(state, -1) << std::endl;

		lua_pop(state, 1);
	}
}

} // namespace crown