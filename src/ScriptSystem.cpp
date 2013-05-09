#include "ScriptSystem.h"
#include <cassert>

namespace crown
{

//-----------------------------------------------------------
LuaState::LuaState()
{
	m_state = luaL_newstate();

    luaL_openlibs(m_state);
}

//-----------------------------------------------------------
LuaState::~LuaState()
{
	if (m_instance != NULL)
	{
		lua_close(m_state);
		delete m_instance;
	}
}

//-----------------------------------------------------------
LuaState* LuaState::instance()
{
	if (m_instance == NULL)
	{
		m_instance = new LuaState();
	}

	return m_instance;
}

//-----------------------------------------------------------
int32_t LuaState::load_buffer(const char* buf, size_t len)
{
	int32_t s = luaL_loadbuffer(m_state, buf, len, "");

	return s;
}

//-----------------------------------------------------------
int32_t LuaState::load_string(const char* str)
{
	int32_t s = luaL_loadstring(m_state, str);

	return s;
}

//-----------------------------------------------------------
int32_t LuaState::execute()
{
    int32_t s = lua_pcall(m_state, 0, LUA_MULTRET, 0);

    return s;
}


//-----------------------------------------------------------
ScriptSystem::ScriptSystem() :
	m_script_count(0)
{
}

//-----------------------------------------------------------
ScriptSystem::~ScriptSystem()
{
	// FIXME
}

//-----------------------------------------------------------
ScriptId ScriptSystem::load(ScriptResource* script)
{
	assert(LuaState::instance()->load_buffer((char*)script->data(), script->length()) == 0);

	ScriptId id;
	id.index = m_script_count;
	id.id = 0;

	m_script[id.index].id = id;
	m_script[id.index].script_resource = script;

	m_script_count++;

	return id;
}

//-----------------------------------------------------------
void ScriptSystem::execute()
{
	assert(LuaState::instance()->execute() == 0);	
}

//-----------------------------------------------------------
void unload(ScriptResource* resource)
{
	(void*) resource;
	// FIXME
}


} // namespace crown