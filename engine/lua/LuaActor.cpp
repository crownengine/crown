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

#include "LuaStack.h"
#include "LuaEnvironment.h"
#include "Actor.h"

namespace crown
{

//-----------------------------------------------------------------------------
CE_EXPORT int actor_enable_gravity(lua_State* L)
{
	LuaStack stack(L);
	Actor* actor = stack.get_actor(1);

	actor->enable_gravity();

	return 0;
}

//-----------------------------------------------------------------------------
CE_EXPORT int actor_disable_gravity(lua_State* L)
{
	LuaStack stack(L);
	Actor* actor = stack.get_actor(1);

	actor->disable_gravity();
	return 0;
}

//-----------------------------------------------------------------------------
CE_EXPORT int actor_enable_collision(lua_State* L)
{
	LuaStack stack(L);
	Actor* actor = stack.get_actor(1);

	actor->enable_collision();

	return 0;
}

//-----------------------------------------------------------------------------
CE_EXPORT int actor_disable_collision(lua_State* L)
{
	LuaStack stack(L);
	Actor* actor = stack.get_actor(1);

	actor->disable_collision();

	return 0;
}

//-----------------------------------------------------------------------------
CE_EXPORT int actor_is_static(lua_State* L)
{
	LuaStack stack(L);

	Actor* actor = stack.get_actor(1);

	stack.push_bool(actor->is_static());
	return 1;
}

//-----------------------------------------------------------------------------
CE_EXPORT int actor_is_dynamic(lua_State* L)
{
	LuaStack stack(L);

	Actor* actor = stack.get_actor(1);

	stack.push_bool(actor->is_dynamic());
	return 1;
}

//-----------------------------------------------------------------------------
CE_EXPORT int actor_linear_damping(lua_State* L)
{
	LuaStack stack(L);

	Actor* actor = stack.get_actor(1);

	stack.push_float(actor->linear_damping());
	return 1;
}

//-----------------------------------------------------------------------------
CE_EXPORT int actor_set_linear_damping(lua_State* L)
{
	LuaStack stack(L);

	Actor* actor = stack.get_actor(1);
	const float rate = stack.get_float(2);

	actor->set_linear_damping(rate);
	return 0;
}

//-----------------------------------------------------------------------------
CE_EXPORT int actor_angular_damping(lua_State* L)
{
	LuaStack stack(L);

	Actor* actor = stack.get_actor(1);

	stack.push_float(actor->angular_damping());
	return 1;
}

//-----------------------------------------------------------------------------
CE_EXPORT int actor_set_angular_damping(lua_State* L)
{
	LuaStack stack(L);

	Actor* actor = stack.get_actor(1);
	const float rate = stack.get_float(2);

	actor->set_angular_damping(rate);
	return 0;
}

//-----------------------------------------------------------------------------
CE_EXPORT int actor_linear_velocity(lua_State* L)
{
	LuaStack stack(L);

	Actor* actor = stack.get_actor(1);

	stack.push_vector3(actor->linear_velocity());
	return 1;
}

//-----------------------------------------------------------------------------
CE_EXPORT int actor_set_linear_velocity(lua_State* L)
{
	LuaStack stack(L);

	Actor* actor = stack.get_actor(1);
	const Vector3& vel = stack.get_vector3(2);

	actor->set_linear_velocity(vel);
	return 0;
}

//-----------------------------------------------------------------------------
CE_EXPORT int actor_angular_velocity(lua_State* L)
{
	LuaStack stack(L);

	Actor* actor = stack.get_actor(1);

	stack.push_vector3(actor->angular_velocity());
	return 1;
}

//-----------------------------------------------------------------------------
CE_EXPORT int actor_set_angular_velocity(lua_State* L)
{
	LuaStack stack(L);

	Actor* actor = stack.get_actor(1);
	const Vector3& vel = stack.get_vector3(2);

	actor->set_angular_velocity(vel);
	return 0;
}

//-----------------------------------------------------------------------------
CE_EXPORT int actor_add_impulse(lua_State* L)
{
	LuaStack stack(L);

	Actor* actor = stack.get_actor(1);
	const Vector3& impulse = stack.get_vector3(2);	

	actor->add_impulse(impulse);

	return 0;
}

//-----------------------------------------------------------------------------
CE_EXPORT int actor_add_impulse_at(lua_State* L)
{
	LuaStack stack(L);

	Actor* actor = stack.get_actor(1);
	const Vector3& impulse = stack.get_vector3(2);
	const Vector3& pos = stack.get_vector3(3);

	actor->add_impulse_at(impulse, pos);

	return 0;
}

//-----------------------------------------------------------------------------
CE_EXPORT int actor_push(lua_State* L)
{
	LuaStack stack(L);

	Actor* actor = stack.get_actor(1);
	const Vector3& vel = stack.get_vector3(2);
	const float mass = stack.get_float(3);

	actor->push(vel, mass);

	return 0;
}

//-----------------------------------------------------------------------------
CE_EXPORT int actor_is_sleeping(lua_State* L)
{
	LuaStack stack(L);

	Actor* actor = stack.get_actor(1);

	stack.push_bool(actor->is_sleeping());
	return 1;
}

//-----------------------------------------------------------------------------
CE_EXPORT int actor_wake_up(lua_State* L)
{
	LuaStack stack(L);

	Actor* actor = stack.get_actor(1);

	actor->wake_up();
	return 0;
}

//-----------------------------------------------------------------------------
void load_actor(LuaEnvironment& env)
{
	env.load_module_function("Actor", "enable_gravity",			actor_enable_gravity);
	env.load_module_function("Actor", "disable_gravity",		actor_disable_gravity);
	env.load_module_function("Actor", "enable_collision",		actor_enable_collision);
	env.load_module_function("Actor", "disable_collision",		actor_disable_collision);
	env.load_module_function("Actor", "is_static",				actor_is_static);
	env.load_module_function("Actor", "is_dynamic",				actor_is_dynamic);
	env.load_module_function("Actor", "linear_damping",			actor_linear_damping);
	env.load_module_function("Actor", "set_linear_damping",		actor_set_linear_damping);
	env.load_module_function("Actor", "angular_damping",		actor_angular_damping);
	env.load_module_function("Actor", "set_angular_damping",	actor_set_angular_damping);
	env.load_module_function("Actor", "linear_velocity",		actor_linear_velocity);
	env.load_module_function("Actor", "set_linear_velocity",	actor_set_linear_velocity);
	env.load_module_function("Actor", "angular_velocity",		actor_angular_velocity);
	env.load_module_function("Actor", "set_angular_velocity",	actor_set_angular_velocity);
	env.load_module_function("Actor", "add_impulse",			actor_add_impulse);
	env.load_module_function("Actor", "add_impulse_at",			actor_add_impulse_at);
	env.load_module_function("Actor", "push",					actor_push);
	env.load_module_function("Actor", "is_sleeping",			actor_is_sleeping);
	env.load_module_function("Actor", "wake_up",				actor_wake_up);
}

} // namespace crown
