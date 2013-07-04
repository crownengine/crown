#include "Crown.h"
#include "Game.h"
#include "lua.hpp"
#include "ConsoleServer.h"

namespace crown
{

StringSetting g_boot("boot_file", "lua main file", "lua/game.raw");


void init()
{
	LuaEnvironment* env = device()->lua_environment();

	const char* path = device()->filesystem()->os_path(g_boot.value());

	env->load_file(path);
	env->execute(0, 0);

	env->get_global_symbol("init");
	env->execute(0, 0);
}

void shutdown()
{
	LuaEnvironment* env = device()->lua_environment();

	env->get_global_symbol("shutdown");
	env->execute(0, 0);
}

void frame(float dt)
{
	LuaEnvironment* env = device()->lua_environment();

	env->get_global_symbol("frame");
	env->stack().push_float(dt);
	env->execute(1, 0);
}

}
