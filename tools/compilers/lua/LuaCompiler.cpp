#include "LuaCompiler.h"
#include "FileStream.h"
#include "lua.hpp"

namespace crown
{

//-----------------------------------------------------------------------------
LuaCompiler::LuaCompiler(const char* root_path, const char* dest_path, const char* resource, uint32_t seed) :
	Compiler(root_path, dest_path, resource, seed),
	m_file_size(0),
	m_file_data(NULL)
{

}

//-----------------------------------------------------------------------------
bool LuaCompiler::compile()
{
	int32_t status;
    lua_State *L;

    luaL_openlibs(L); // Load Lua libraries 

    char* file;
    strcpy(file, root_path());
    strcat(file, resource_path());

    /* Load the file containing the script we are going to run */
    status = luaL_loadfile(L, file);

    if (status)
    {
        printf("Couldn't load file: %s\n", lua_tostring(L, -1));
        return -1;
    }
    else
    {
    	printf("yeah!\n");
    }

	return true;
}

//-----------------------------------------------------------------------------
void LuaCompiler::write()
{

}

} // namespace crown
