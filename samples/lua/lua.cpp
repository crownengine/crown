#include <iostream>
#include "Crown.h"
#include "lua.hpp"
#include <unistd.h>

using namespace crown;

static void report_errors(lua_State* state, const int status)
{
  if (status != 0)
  {
    std::cerr << "-- " << lua_tostring(state, -1) << std::endl;

    lua_pop(state, 1);
  }
}

int main(int argc, char** argv)
{
  lua_State* lua_state;

  Filesystem fs("/home/mikymod/test/res_compiled");

  FileResourceArchive archive(fs);

  MallocAllocator allocator;

  ResourceManager res_manager(archive, allocator);

  ResourceId script = res_manager.load("lua/hello.lua");

  res_manager.flush();

  while (1)
  {
    if (res_manager.is_loaded(script))
    {

      lua_state = luaL_newstate();
      luaL_openlibs(lua_state);

      assert(res_manager.data(script) != NULL);

      ScriptResource* resource = (ScriptResource*)res_manager.data(script);

      int s = luaL_loadbuffer(lua_state, (char*)resource->data(), 47, "");

      if (s == 0)
      {
        s = lua_pcall(lua_state, 0, LUA_MULTRET, 0);
      }

      report_errors(lua_state, s);

      break;
    }
  }

  lua_close(lua_state);

  return 0;
}
