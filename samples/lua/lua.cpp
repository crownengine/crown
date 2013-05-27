#include "Crown.h"
#include "Game.h"


namespace crown
{

lua_State* L;

void init()
{
	L = luaL_newstate();
	luaL_openlibs(L);

	lua_cpcall(L, luaopen_libcrownlua, NULL);

	if (luaL_loadfile(L, "/home/mikymod/test/res_linux/lua/game.raw") || lua_pcall(L, 0, 0, 0))
	{
		os::printf("error: %s", lua_tostring(L, -1));
	}

	lua_getglobal(L, "init");

	lua_pcall(L, 0, 0, 0);
}

void shutdown()
{
	lua_getglobal(L, "shutdown");

	lua_pcall(L, 0, 0, 0);

	lua_close(L);
}

void frame(float dt)
{
	lua_getglobal(L, "frame");

	lua_pushnumber(L, dt);

	lua_pcall(L, 1, 0, 0);
}

}
