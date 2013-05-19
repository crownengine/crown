#pragma once

#include "lua.hpp"

#include "Types.h"
#include "String.h"

#include "Vec2.h"
#include "Vec3.h"
#include "Mat4.h"
#include "Quat.h"


namespace crown
{

/// Abstraction of lua state.
/// It provides 2 ways for loading lua code, as a buffer or as a string.
/// It must be included only by ScriptSystem. 
class LuaState
{
public:
	/// Constructor, private for singleton
								LuaState();
	/// Destructor
								~LuaState();
	/// Load lua chunk as buffer
	int32_t 					load_buffer(const char* buf, size_t len);
	/// Load lua chunk as string
	int32_t						load_string(const char* str);
	/// Load lua chunk as file
	int32_t 					load_file(const char* file);
	/// Executes lua chunk loaded in stack
	int32_t 					execute();

private:

								/// Lua state incapsulated by this class
	lua_State*					m_state;
};

/// ScriptSystem allows to execute lua code or bytecode chunks
/// It also provides some utilities for crown-lua environment
class ScriptSystem
{
public:
								/// Constructor
								ScriptSystem();

								/// Loads script resource
	void						load(const char* script);
								/// Executes
	void						execute();
								/// Unloads script resource
	void						unload(const char* script);
								/// Returns the first free Vec2
	Vec2&						next_vec2(float nx, float ny);
								/// Returns the first free Vec3
	Vec3&						next_vec3(float nx, float ny, float nz);
								/// Returns the first free Mat4
	Mat4&						next_mat4(float r1c1, float r2c1, float r3c1, float r1c2, float r2c2, float r3c2, float r1c3, float r2c3, float r3c3);	
								/// Returns the first free Quat
	Quat&						next_quat(float angle, const Vec3& v);
								/// Returns the number of vec2 used in lua environment
	uint32_t					vec2_used();
								/// Returns the number of vec3 used in lua environment
	uint32_t					vec3_used();
								/// Returns the number of mat4 used in lua environment
	uint32_t					mat4_used();
								/// Returns the number of quat used in lua environment
	uint32_t 					quat_used();

								/// First file loaded by ScriptSystem
	static const char*			BOOT_SCRIPT;
								/// Max number of temporary objects allowed
	static const uint32_t		MAX_TEMP_OBJECTS 	= 1024;


private:
	LuaState 					m_state;							
								/// Vec2 used by lua environment
	Vec2 						m_vec2_list[MAX_TEMP_OBJECTS];
								/// Counter which points to the next free Vec2
	uint32_t					m_vec2_count;
								/// Vec3 used by lua environment
	Vec3						m_vec3_list[MAX_TEMP_OBJECTS];
								/// Counter which points to the next free Vec3
	uint32_t					m_vec3_count;
								/// Mat4 used by lua environment
	Mat4						m_mat4_list[MAX_TEMP_OBJECTS];
								/// Counter which points to the next free Mat4
	uint32_t					m_mat4_count;
								/// Quaternions used by lua environment
	Quat						m_quat_list[MAX_TEMP_OBJECTS];
								/// Counter which points to the nect free Quat
	uint32_t					m_quat_count;
};


// This block provides fews utilities for lua environment
extern "C"
{
	uint32_t script_system_vec2_used();

	uint32_t script_system_vec3_used();

	uint32_t script_system_mat4_used();

	uint32_t script_system_quat_used();
}

} // namespace crown