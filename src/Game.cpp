#include "Crown.h"
#include "Game.h"
#include "lua.hpp"
#include "ConsoleServer.h"

namespace crown
{

StringSetting g_boot("boot_file", "lua main file", "lua/game.raw");

ConsoleServer server;

void init()
{
	// L = luaL_newstate();
	// luaL_openlibs(L);

	// lua_cpcall(L, luaopen_libcrown, NULL);

	// if (luaL_loadfile(L, path) || lua_pcall(L, 0, 0, 0))
	// {
	// 	os::printf("error: %s", lua_tostring(L, -1));
	// }

	// lua_getglobal(L, "init");

	// lua_pcall(L, 0, 0, 0);

	LuaEnvironment* env = device()->lua_environment();

	const char* path = device()->filesystem()->os_path(g_boot.value());

	env->load_file(path);
	env->execute(0, 0);

	env->get_global_symbol("init");
	env->execute(0, 0);

	server.start(10000);
}

void shutdown()
{
	// lua_getglobal(L, "shutdown");

	// lua_pcall(L, 0, 0, 0);

	// lua_close(L);

	LuaEnvironment* env = device()->lua_environment();

	env->get_global_symbol("shutdown");
	env->execute(0, 0);

	server.stop();
}

void frame(float dt)
{
	// lua_getglobal(L, "frame");

	// lua_pushnumber(L, dt);

	// lua_pcall(L, 1, 0, 0);

	LuaEnvironment* env = device()->lua_environment();

	env->get_global_symbol("frame");
	env->stack().push_float(dt);
	env->execute(1, 0);

	server.receive_command();
}

}
