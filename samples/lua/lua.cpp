#include "Crown.h"
#include "Game.h"


namespace crown
{

StringSetting g_boot("boot_file", "lua main file", "lua/game.raw");

lua_State* L;

void init()
{/*
	L = luaL_newstate();
	luaL_openlibs(L);

	lua_cpcall(L, luaopen_libcrownlua, NULL);

	const char* path = device()->filesystem()->os_path(g_boot.value());

	if (luaL_loadfile(L, path) || lua_pcall(L, 0, 0, 0))
	{
		os::printf("error: %s", lua_tostring(L, -1));
	}

	lua_getglobal(L, "init");

	lua_pcall(L, 0, 0, 0);
	*/
}

void shutdown()
{
	/*
	lua_getglobal(L, "shutdown");

	lua_pcall(L, 0, 0, 0);

	lua_close(L);
	*/
}

void frame(float dt)
{
	/*
	lua_getglobal(L, "frame");

	lua_pushnumber(L, dt);

	lua_pcall(L, 1, 0, 0);
	*/
}

}
