#include <iostream>
#include "Crown.h"
#include <unistd.h>
#include "ScriptSystem.h"

using namespace crown;

int main(int argc, char** argv)
{
  lua_State* lua_state;

  Filesystem fs("/home/mikymod/test/res_compiled");

  FileResourceArchive archive(fs);

  MallocAllocator allocator;

  ResourceManager res_manager(archive, allocator);

  ResourceId script = res_manager.load("lua/init.lua");

  res_manager.flush();

  ScriptSystem* sys = scripter();

  while (1)
  {
    if (res_manager.is_loaded(script))
    {
      assert(res_manager.data(script) != NULL);

      ScriptResource* resource = (ScriptResource*)res_manager.data(script);

      sys->load(resource);
      sys->execute();

      break;
    }
  }

  return 0;
}
