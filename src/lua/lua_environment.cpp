/*
 * Copyright (c) 2012-2015 Daniele Bartolini and individual contributors.
 * License: https://github.com/taylor001/crown/blob/master/LICENSE
 */


#include "config.h"
#include "error.h"
#include "lua_environment.h"
#include "lua_stack.h"
#include "lua_resource.h"
#include "device.h"
#include "resource_manager.h"
#include "log.h"
#include <stdarg.h>

namespace crown
{

// Lua modules
extern void load_actor(LuaEnvironment& env);
extern void load_camera(LuaEnvironment& env);
extern void load_controller(LuaEnvironment& env);
extern void load_debug_line(LuaEnvironment& env);
extern void load_device(LuaEnvironment& env);
extern void load_gui(LuaEnvironment& env);
extern void load_math(LuaEnvironment& env);
extern void load_physics_world(LuaEnvironment& env);
extern void load_raycast(LuaEnvironment& env);
extern void load_resource_package(LuaEnvironment& env);
extern void load_sound_world(LuaEnvironment& env);
extern void load_sprite(LuaEnvironment& env);
extern void load_settings(LuaEnvironment& env);
extern void load_unit(LuaEnvironment& env);
extern void load_window(LuaEnvironment& env);
extern void load_world(LuaEnvironment& env);
extern void load_material(LuaEnvironment& env);
extern void load_input(LuaEnvironment& env);

// When an error occurs, logs the error message and pauses the engine.
static int error_handler(lua_State* L)
{
	lua_getfield(L, LUA_GLOBALSINDEX, "debug");
	if (!lua_istable(L, -1))
	{
		lua_pop(L, 1);
		return 0;
	}

	lua_getfield(L, -1, "traceback");
	if (!lua_isfunction(L, -1))
	{
		lua_pop(L, 2);
		return 0;
	}

	lua_pushvalue(L, 1); // Pass error message
	lua_pushinteger(L, 2);
	lua_call(L, 2, 1); // Call debug.traceback

	CE_LOGE(lua_tostring(L, -1)); // Print error message
	lua_pop(L, 1); // Remove error message from stack
	lua_pop(L, 1); // Remove debug.traceback from stack

	device()->pause();
	return 0;
}

// Redirects require to the resource manager.
static int require(lua_State* L)
{
	using namespace lua_resource;
	LuaStack stack(L);
	const LuaResource* lr = (LuaResource*)device()->resource_manager()->get(SCRIPT_TYPE, stack.get_resource_id(1));
	luaL_loadbuffer(L, program(lr), size(lr), "");
	return 1;
}

static int lightuserdata_add(lua_State* L)
{
	LuaStack stack(L);
	const Vector3& a = stack.get_vector3(1);
	const Vector3& b = stack.get_vector3(2);
	stack.push_vector3(a + b);
	return 1;
}

static int lightuserdata_sub(lua_State* L)
{
	LuaStack stack(L);
	const Vector3& a = stack.get_vector3(1);
	const Vector3& b = stack.get_vector3(2);
	stack.push_vector3(a - b);
	return 1;
}

static int lightuserdata_mul(lua_State* L)
{
	LuaStack stack(L);
	const Vector3& a = stack.get_vector3(1);
	const float b = stack.get_float(2);
	stack.push_vector3(a * b);
	return 1;
}

static int lightuserdata_div(lua_State* L)
{
	LuaStack stack(L);
	const Vector3& a = stack.get_vector3(1);
	const float b = stack.get_float(2);
	stack.push_vector3(a / b);
	return 1;
}

static int lightuserdata_unm(lua_State* L)
{
	LuaStack stack(L);
	stack.push_vector3(-stack.get_vector3(1));
	return 1;
}

static int lightuserdata_index(lua_State* L)
{
	LuaStack stack(L);
	Vector3& v = stack.get_vector3(1);
	const char* s = stack.get_string(2);

	switch (s[0])
	{
		case 'x': stack.push_float(v.x); return 1;
		case 'y': stack.push_float(v.y); return 1;
		case 'z': stack.push_float(v.z); return 1;
		default: LUA_ASSERT(false, stack, "Bad index: '%c'", s[0]); break;
	}

	return 0;
}

static int lightuserdata_newindex(lua_State* L)
{
	LuaStack stack(L);
	Vector3& v = stack.get_vector3(1);
	const char* s = stack.get_string(2);
	const float value = stack.get_float(3);

	switch (s[0])
	{
		case 'x': v.x = value; break;
		case 'y': v.y = value; break;
		case 'z': v.z = value; break;
		default: LUA_ASSERT(false, stack, "Bad index: '%c'", s[0]); break;
	}

	return 0;
}

LuaEnvironment::LuaEnvironment()
	: L(NULL)
	, _vec3_used(0)
	, _quat_used(0)
	, _mat4_used(0)
{
	L = luaL_newstate();
	CE_ASSERT(L, "Unable to create lua state");
}

LuaEnvironment::~LuaEnvironment()
{
	lua_close(L);
}

void LuaEnvironment::load_libs()
{
	lua_gc(L, LUA_GCSTOP, 0);

	// Open default libraries
	luaL_openlibs(L);

	// Register crown libraries
	load_actor(*this);
	load_camera(*this);
	load_controller(*this);
	load_debug_line(*this);
	load_device(*this);
	load_gui(*this);
	load_math(*this);
	load_physics_world(*this);
	load_raycast(*this);
	load_resource_package(*this);
	load_sound_world(*this);
	load_sprite(*this);
	load_settings(*this);
	load_unit(*this);
	load_window(*this);
	load_world(*this);
	load_material(*this);
	load_input(*this);

	// Register custom loader
	lua_getfield(L, LUA_GLOBALSINDEX, "package");
	lua_getfield(L, -1, "loaders");
	lua_remove(L, -2);

	int num_loaders = 0;
	lua_pushnil(L);
	while (lua_next(L, -2) != 0)
	{
		lua_pop(L, 1);
		num_loaders++;
	}
	lua_pushinteger(L, num_loaders + 1);
	lua_pushcfunction(L, require);
	lua_rawset(L, -3);
	lua_pop(L, 1);

	// Create metatable for lightuserdata
	luaL_newmetatable(L, "Lightuserdata_mt");
	lua_pushstring(L, "__add");
	lua_pushcfunction(L, lightuserdata_add);
	lua_settable(L, 1);

	lua_pushstring(L, "__sub");
	lua_pushcfunction(L, lightuserdata_sub);
	lua_settable(L, 1);

	lua_pushstring(L, "__mul");
	lua_pushcfunction(L, lightuserdata_mul);
	lua_settable(L, 1);

	lua_pushstring(L, "__div");
	lua_pushcfunction(L, lightuserdata_div);
	lua_settable(L, 1);

	lua_pushstring(L, "__unm");
	lua_pushcfunction(L, lightuserdata_unm);
	lua_settable(L, 1);

	lua_pushstring(L, "__index");
	lua_pushcfunction(L, lightuserdata_index);
	lua_settable(L, 1);

	lua_pushstring(L, "__newindex");
	lua_pushcfunction(L, lightuserdata_newindex);
	lua_settable(L, 1);

	lua_pop(L, 1); // Pop Lightuserdata_mt

	// Ensure stack is clean
	CE_ASSERT(lua_gettop(L) == 0, "Stack not clean");

	lua_gc(L, LUA_GCRESTART, 0);
}

void LuaEnvironment::execute(const LuaResource* lr)
{
	using namespace lua_resource;
	lua_pushcfunction(L, error_handler);
	luaL_loadbuffer(L, program(lr), size(lr), "<unknown>");
	lua_pcall(L, 0, 0, -2);
	lua_pop(L, 1);
}

void LuaEnvironment::execute_string(const char* s)
{
	lua_pushcfunction(L, error_handler);
	luaL_loadstring(L, s);
	lua_pcall(L, 0, 0, -2);
	lua_pop(L, 1);
}

void LuaEnvironment::load_module_function(const char* module, const char* name, const lua_CFunction func)
{
	luaL_newmetatable(L, module);
	luaL_Reg entry[2];

	entry[0].name = name;
	entry[0].func = func;
	entry[1].name = NULL;
	entry[1].func = NULL;

	luaL_register(L, NULL, entry);
	lua_setglobal(L, module);
	lua_pop(L, -1);
}

void LuaEnvironment::load_module_function(const char* module, const char* name, const char* value)
{
	luaL_newmetatable(L, module);
	lua_getglobal(L, value);
	lua_setfield(L, -2, name);
	lua_setglobal(L, module);
}

void LuaEnvironment::load_module_constructor(const char* module, const lua_CFunction func)
{
	// Create dummy tables to be used as module's metatable
	lua_createtable(L, 0, 1);
	lua_pushstring(L, "__call");
	lua_pushcfunction(L, func);
	lua_settable(L, 1); // dummy.__call = func
	lua_getglobal(L, module);
	lua_pushvalue(L, -2); // Duplicate dummy metatable
	lua_setmetatable(L, -2); // setmetatable(module, dummy)
	lua_pop(L, -1);
}

void LuaEnvironment::load_module_enum(const char* module, const char* name, uint32_t value)
{
	// Checks table existance
	lua_pushstring(L, module);
	lua_rawget(L, LUA_GLOBALSINDEX);
	if (!lua_istable(L, -1)) // If not exixts
	{
		// Creates table
		lua_newtable(L);
		lua_setglobal(L, module);
	}

	// Adds field to table
	lua_getglobal(L, module);
	lua_pushinteger(L, value);
	lua_setfield(L, -2, name);
	lua_pop(L, 2);
}

void LuaEnvironment::call_global(const char* func, uint8_t argc, ...)
{
	CE_ASSERT_NOT_NULL(func);

	LuaStack stack(L);

	va_list vl;
	va_start(vl, argc);

	lua_pushcfunction(L, error_handler);
	lua_getglobal(L, func);

	for (uint8_t i = 0; i < argc; i++)
	{
		const int type = va_arg(vl, int);
		switch (type)
		{
			case ARGUMENT_FLOAT:
			{
				stack.push_float(va_arg(vl, double));
				break;
			}
			default:
			{
				CE_ASSERT(false, "Oops, lua argument unknown");
				break;
			}
		}
	}

	va_end(vl);
	lua_pcall(L, argc, 0, -argc - 2);
	lua_pop(L, -1);
}

void LuaEnvironment::call_physics_callback(Actor* actor_0, Actor* actor_1, Unit* unit_0, Unit* unit_1, const Vector3& where, const Vector3& normal, const char* type)
{
	LuaStack stack(L);

	lua_pushcfunction(L, error_handler);
	lua_getglobal(L, "g_physics_callback");

	stack.push_table();
	stack.push_key_begin("actor_0"); (actor_0 ? stack.push_actor(actor_0) : stack.push_nil()); stack.push_key_end();
	stack.push_key_begin("actor_1"); (actor_1 ? stack.push_actor(actor_1) : stack.push_nil()); stack.push_key_end();
	stack.push_key_begin("unit_0"); (unit_0 ? stack.push_unit(unit_0) : stack.push_nil()); stack.push_key_end();
	stack.push_key_begin("unit_1"); (unit_1 ? stack.push_unit(unit_1) : stack.push_nil()); stack.push_key_end();
	stack.push_key_begin("where"); stack.push_vector3(where); stack.push_key_end();
	stack.push_key_begin("normal"); stack.push_vector3(normal); stack.push_key_end();
	stack.push_key_begin("type"); stack.push_string(type); stack.push_key_end();

	lua_pcall(L, 1, 0, -3);
	lua_pop(L, -1);
}

void LuaEnvironment::call_trigger_callback(Actor* trigger, Actor* other, const char* type)
{
	LuaStack stack(L);

	lua_pushcfunction(L, error_handler);
	lua_getglobal(L, "g_trigger_callback");

	stack.push_table();
	stack.push_key_begin("trigger"); (trigger ? stack.push_actor(trigger) : stack.push_nil()); stack.push_key_end();
	stack.push_key_begin("other"); (other ? stack.push_actor(other) : stack.push_nil()); stack.push_key_end();
	stack.push_key_begin("type"); stack.push_string(type); stack.push_key_end();

	lua_pcall(L, 1, 0, -3);
	lua_pop(L, -1);
}

Vector3* LuaEnvironment::next_vector3(const Vector3& v)
{
	CE_ASSERT(_vec3_used < CROWN_MAX_LUA_VECTOR3, "Maximum number of Vector3 reached");

	return &(_vec3_buffer[_vec3_used++] = v);
}

Matrix4x4* LuaEnvironment::next_matrix4x4(const Matrix4x4& m)
{
	CE_ASSERT(_mat4_used < CROWN_MAX_LUA_MATRIX4X4, "Maximum number of Matrix4x4 reached");

	return &(s_mat4_buffer[_mat4_used++] = m);
}

Quaternion* LuaEnvironment::next_quaternion(const Quaternion& q)
{
	CE_ASSERT(_quat_used < CROWN_MAX_LUA_QUATERNION, "Maximum number of Quaternion reached");
	return &(_quat_buffer[_quat_used++] = q);
}

bool LuaEnvironment::is_vector3(int index)
{
	void* type = lua_touserdata(L, index);
	return (type >= &_vec3_buffer[0] && type <= &_vec3_buffer[CROWN_MAX_LUA_VECTOR3 - 1]);
}

bool LuaEnvironment::is_matrix4x4(int index)
{
	void* type = lua_touserdata(L, index);
	return (type >= &s_mat4_buffer[0] && type <= &s_mat4_buffer[CROWN_MAX_LUA_MATRIX4X4 - 1]);
}

bool LuaEnvironment::is_quaternion(int index)
{
	void* type = lua_touserdata(L, index);
	return (type >= &_quat_buffer[0] && type <= &_quat_buffer[CROWN_MAX_LUA_QUATERNION - 1]);
}

void LuaEnvironment::clear_temporaries()
{
	_vec3_used = 0;
	_mat4_used = 0;
	_quat_used = 0;
}

} // namespace crown
