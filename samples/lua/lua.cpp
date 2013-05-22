#include "lua.hpp"
#include "Crown.h"
#include "Game.h"


namespace crown
{

lua_State* state;
int z;

void init()
{
	state = luaL_newstate();
	luaL_openlibs(state);

	if (luaL_loadfile(state, "lua_sample/lua/game.raw") || lua_pcall(state, 0, 0, 0))
	{
		os::printf("error: %s", lua_tostring(state, -1));
	}

	lua_getglobal(state, "init");

	lua_pcall(state, 0, 0, 0);
}

void shutdown()
{
	lua_getglobal(state, "shutdown");

	lua_pcall(state, 0, 0, 0);

	lua_close(state);
}

void frame(float dt)
{
	lua_getglobal(state, "frame");

	lua_pushnumber(state, dt);

	lua_pcall(state, 1, 0, 0);
}


}