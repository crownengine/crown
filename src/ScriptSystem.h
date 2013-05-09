#pragma once

#include "Types.h"
#include "String.h"
#include "lua.hpp"
#include "ScriptResource.h"

namespace crown
{

//------------------------------------------------------------------
struct ScriptId
{
	uint16_t	index;
	uint16_t	id;
};

//------------------------------------------------------------------
struct Script
{
	ScriptId		id;
	ScriptResource* script_resource;
};

/// Singleton class that abstracts lua state
class LuaState
{
public:
							/// Destructor
							~LuaState();
							/// Get singleton instance
	static LuaState*		instance();
							/// Load lua chunk as buffer
	int32_t 				load_buffer(const char* buf, size_t len);
							/// Load lua chunk as string
	int32_t					load_string(const char* str);
							/// Executes lua chunk loaded in stack
	int32_t 				execute();

private:
							/// Constructor, private for singleton
							LuaState();
							/// Lua state incapsulated by this class
	lua_State*				m_state;
							/// LuaState pointer for singleton
	static LuaState* 		m_instance;
};

LuaState* LuaState::m_instance = NULL;

/// Script System allows to execute lua code or lua bytecode chunk
class ScriptSystem
{
public:
							/// Max number of scripts which can be loaded by this system
	static const int32_t	MAX_SCRIPTS = 256;


							/// Constructor
							ScriptSystem();
							///	Destructor
							~ScriptSystem();
							/// Load scripr resource
	ScriptId				load(ScriptResource* script);
							/// Execute 
	void					execute();
							/// Unload script resource
	void					unload(ScriptResource* script);

private:
							/// Number of scripts already loaded
	uint32_t				m_script_count;
							/// Resource handle
	Script					m_script[MAX_SCRIPTS];

	friend class ScriptResource;
};

} // namespace crown