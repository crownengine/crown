#include <iostream>

#include "lua.hpp"

void report_errors(lua_State *L, int status)
{
  if ( status!=0 ) {
    std::cerr << "-- " << lua_tostring(L, -1) << std::endl;
    lua_pop(L, 1); // remove error message
  }
}

int main(int argc, char** argv)
{
  for ( int n=1; n<argc; ++n ) {
    const char* file = argv[n];

    lua_State *L = luaL_newstate();

	luaL_openlibs(L);

    std::cerr << "-- Loading file: " << file << std::endl;

    int s = luaL_loadfile(L, file);

    if ( s==0 ) {
      // execute Lua program
      s = lua_pcall(L, 0, LUA_MULTRET, 0);
    }

    report_errors(L, s);
    lua_close(L);
    std::cerr << std::endl;
  }

  return 0;
}
