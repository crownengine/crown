#include "lua.hpp"
#include "Device.h"
/*
#include "Game.h"

namespace crown
{

lua_State* state;

void init()
{
	state = luaL_newstate();
	luaL_openlibs(state);

	luaL_loadfile(state, "lua/lua/game.lua.script");

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
*/