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
	return true;
}

//-----------------------------------------------------------------------------
void LuaCompiler::write()
{

}

} // namespace crown
