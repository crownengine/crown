/*
Copyright (c) 2013 Daniele Bartolini, Michele Rossi
Copyright (c) 2012 Daniele Bartolini, Simone Boscaratto

Permission is hereby granted, free of charge, to any person
obtaining a copy of this software and associated documentation
files (the "Software"), to deal in the Software without
restriction, including without limitation the rights to use,
copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the
Software is furnished to do so, subject to the following
conditions:

The above copyright notice and this permission notice shall be
included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
OTHER DEALINGS IN THE SOFTWARE.
*/

#pragma once

#include "config.h"
#include "types.h"
#include "macros.h"
#include "math_types.h" // HACK
#include "lua.hpp"

namespace crown
{

enum LuaArgumentType
{
	ARGUMENT_FLOAT
};

struct LuaResource;
// HACK
struct Actor;
struct Unit;

/// LuaEnvironment is a wrapper of a subset of Lua functions and
/// provides utilities for extending Lua
struct LuaEnvironment
{
	LuaEnvironment(lua_State* L);

	void execute(const LuaResource* lr);

	/// Loads and executes the given @a s lua string.
	void execute_string(const char* s);

	/// Loads the function with the given @a name and @a func into the table @a module.
	void load_module_function(const char* module, const char* name, const lua_CFunction func);
	void load_module_function(const char* module, const char* name, const char* value);
	void load_module_constructor(const char* module, const lua_CFunction func);

	/// Loads the enum with the given @a name and @a value into the table @a module.
	void load_module_enum(const char* module, const char* name, uint32_t value);

	/// Calls the global function @a func with @a argc argument number.
	/// Each argument is a pair (type, value).
	/// Example call:
	/// call_global("myfunc", 1, ARGUMENT_FLOAT, 3.14f)
	/// Returns true if success, false otherwise
	void call_global(const char* func, uint8_t argc, ...);

	// HACK
	void call_physics_callback(Actor* actor_0, Actor* actor_1, Unit* unit_0, Unit* unit_1, const Vector3& where, const Vector3& normal, const char* type);
	void call_trigger_callback(Actor* trigger, Actor* other, const char* type);

private:

	lua_State* _L;

private:

	// Disable copying
	LuaEnvironment(const LuaEnvironment&);
	LuaEnvironment& operator=(const LuaEnvironment&);
};

} // namespace crown
