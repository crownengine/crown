#include "ScriptSystem.h"
#include "Filesystem.h"
#include <cassert>

namespace crown
{

const char* BOOT_SCRIPT = "lua/game.lua";

//-----------------------------------------------------------
// ScriptSystem
//-----------------------------------------------------------

//-----------------------------------------------------------
ScriptSystem::ScriptSystem() :
m_state(),
m_vec2_count(0),
m_vec3_count(0),
m_mat4_count(0),
m_quat_count(0)
{
}

//-----------------------------------------------------------
void ScriptSystem::load(const char* script)
{
	assert(m_state.load_file(script) == 0);
}

//-----------------------------------------------------------
void ScriptSystem::execute()
{
	assert(m_state.execute() == 0);	
}

//-----------------------------------------------------------
void ScriptSystem::unload(const char* script)
{
	(void*) script;
}

//-----------------------------------------------------------
Vec2& ScriptSystem::next_vec2(float nx, float ny)
{
	uint32_t current = m_vec2_count;

	m_vec2_list[current].x = nx;
	m_vec2_list[current].y = ny;

	m_vec2_count++;

	return m_vec2_list[current];
}

//-----------------------------------------------------------
Vec3& ScriptSystem::next_vec3(float nx, float ny, float nz)
{
	uint32_t current = m_vec3_count;

	m_vec3_list[current].x = nx;
	m_vec3_list[current].y = ny;
	m_vec3_list[current].z = nz;

	m_vec3_count++;

	return m_vec3_list[current];
}

//-----------------------------------------------------------
Mat4& ScriptSystem::next_mat4(float r1c1, float r2c1, float r3c1, float r1c2, float r2c2, float r3c2, float r1c3, float r2c3, float r3c3)
{
	uint32_t current = m_mat4_count;

	m_mat4_list[current].m[0] 	= r1c1;
	m_mat4_list[current].m[1] 	= r2c1;
	m_mat4_list[current].m[2] 	= r3c1;
	m_mat4_list[current].m[3] 	= 0;
	m_mat4_list[current].m[4] 	= r1c2;
	m_mat4_list[current].m[5] 	= r2c2;
	m_mat4_list[current].m[6] 	= r3c2;
	m_mat4_list[current].m[7] 	= 0;
	m_mat4_list[current].m[8] 	= r1c3;
	m_mat4_list[current].m[9] 	= r2c3;
	m_mat4_list[current].m[10] 	= r3c3;
	m_mat4_list[current].m[11] 	= 0;
	m_mat4_list[current].m[12] 	= 0;
	m_mat4_list[current].m[13] 	= 0;
	m_mat4_list[current].m[14] 	= 0;
	m_mat4_list[current].m[15] 	= 1;

	m_mat4_count++;

	return m_mat4_list[current];
}

//-----------------------------------------------------------
Quat& ScriptSystem::next_quat(float angle, const Vec3& v)
{
	uint32_t current = m_quat_count;

	m_quat_list[current].w = angle;
	m_quat_list[current].v = v;

	m_quat_count++;

	return m_quat_list[current];
}

//-----------------------------------------------------------
uint32_t ScriptSystem::vec2_used()
{
	return m_vec2_count;
}
//-----------------------------------------------------------
uint32_t ScriptSystem::vec3_used()
{
	return m_vec3_count;
}

//-----------------------------------------------------------
uint32_t ScriptSystem::mat4_used()
{
	return m_mat4_count;
}

//-----------------------------------------------------------
uint32_t ScriptSystem::quat_used()
{
	return m_quat_count;
}

//-----------------------------------------------------------
ScriptSystem g_script;

ScriptSystem* scripter()
{
	return &g_script;
}

//-----------------------------------------------------------
// Lua State
//-----------------------------------------------------------

//-----------------------------------------------------------
LuaState::LuaState()
{
	m_state = luaL_newstate();

    luaL_openlibs(m_state);
}

//-----------------------------------------------------------
LuaState::~LuaState()
{
	lua_close(m_state);
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
int32_t LuaState::load_file(const char* file)
{
	int32_t s = luaL_loadfile(m_state, file);

	return s;
}

//-----------------------------------------------------------
int32_t LuaState::execute()
{
    int32_t s = lua_pcall(m_state, 0, LUA_MULTRET, 0);

    return s;
}

//-----------------------------------------------------------
// Extern C
//-----------------------------------------------------------

//-----------------------------------------------------------
uint32_t script_system_vec2_used()
{
	return scripter()->vec2_used();
}

//-----------------------------------------------------------
uint32_t script_system_vec3_used()
{
	return scripter()->vec3_used();
}

//-----------------------------------------------------------
uint32_t script_system_mat4_used()
{
	return scripter()->mat4_used();
}

//-----------------------------------------------------------
uint32_t script_system_quat_used()
{
	return scripter()->quat_used();
}

} // namespace crown