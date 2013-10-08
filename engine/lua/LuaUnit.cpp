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
#include "Unit.h"

namespace crown
{

//-----------------------------------------------------------------------------
CE_EXPORT int32_t unit_local_position(lua_State* L)
{
	LuaStack stack(L);

	Unit* unit = stack.get_unit(1);

	stack.push_vector3(unit->local_position());
	return 1;
}

//-----------------------------------------------------------------------------
CE_EXPORT int32_t unit_local_rotation(lua_State* L)
{
	LuaStack stack(L);

	Unit* unit = stack.get_unit(1);

	stack.push_quaternion(unit->local_rotation());
	return 1;
}

//-----------------------------------------------------------------------------
CE_EXPORT int32_t unit_local_pose(lua_State* L)
{
	LuaStack stack(L);

	Unit* unit = stack.get_unit(1);

	stack.push_matrix4x4(unit->local_pose());
	return 1;
}

//-----------------------------------------------------------------------------
CE_EXPORT int32_t unit_world_position(lua_State* L)
{
	LuaStack stack(L);

	Unit* unit = stack.get_unit(1);

	stack.push_vector3(unit->world_position());
	return 1;
}

//-----------------------------------------------------------------------------
CE_EXPORT int32_t unit_world_rotation(lua_State* L)
{
	LuaStack stack(L);

	Unit* unit = stack.get_unit(1);

	stack.push_quaternion(unit->world_rotation());
	return 1;
}

//-----------------------------------------------------------------------------
CE_EXPORT int32_t unit_world_pose(lua_State* L)
{
	LuaStack stack(L);

	Unit* unit = stack.get_unit(1);

	stack.push_matrix4x4(unit->world_pose());
	return 1;
}

//-----------------------------------------------------------------------------
CE_EXPORT int32_t unit_set_local_position(lua_State* L)
{
	LuaStack stack(L);

	Unit* unit = stack.get_unit(1);
	Vector3& pos = stack.get_vector3(2);

	unit->set_local_position(pos);
	return 0;
}

//-----------------------------------------------------------------------------
CE_EXPORT int32_t unit_set_local_rotation(lua_State* L)
{
	LuaStack stack(L);

	Unit* unit = stack.get_unit(1);
	Quaternion& rot = stack.get_quaternion(2);

	unit->set_local_rotation(rot);
	return 0;
}

//-----------------------------------------------------------------------------
CE_EXPORT int32_t unit_set_local_pose(lua_State* L)
{
	LuaStack stack(L);

	Unit* unit = stack.get_unit(1);
	Matrix4x4& pose = stack.get_matrix4x4(2);

	unit->set_local_pose(pose);
	return 0;
}

//-----------------------------------------------------------------------------
CE_EXPORT void unit_add_component(lua_State* L)
{
	LuaStack stack(L);

	Unit* unit = stack.get_unit(1);
	const char* name = stack.get_string(2);
	uint32_t type = stack.get_int(3);

	ComponentId component;
	component.decode(stack.get_int(4));

	unit->add_component(name, type, component);
}

//-----------------------------------------------------------------------------
CE_EXPORT void unit_remove_component(lua_State* L)
{
	LuaStack stack(L);

	Unit* unit = stack.get_unit(1);
	const char* name = stack.get_string(2);

	unit->remove_component(name);
}

//-----------------------------------------------------------------------------
CE_EXPORT int unit_camera(lua_State* L)
{
	LuaStack stack(L);

	Unit* unit = stack.get_unit(1);

	stack.push_camera(unit->camera("fixme"));
	return 1;
}

//-----------------------------------------------------------------------------
void load_unit(LuaEnvironment& env)
{
	env.load_module_function("Unit", "local_position",			unit_local_position);
	env.load_module_function("Unit", "local_rotation",			unit_local_rotation);
	env.load_module_function("Unit", "local_pose",				unit_local_pose);
	env.load_module_function("Unit", "world_position",			unit_world_position);
	env.load_module_function("Unit", "world_rotation",			unit_world_rotation);
	env.load_module_function("Unit", "world_pose",				unit_local_pose);
	env.load_module_function("Unit", "set_local_position",		unit_set_local_position);
	env.load_module_function("Unit", "set_local_rotation",		unit_set_local_rotation);
	env.load_module_function("Unit", "set_local_pose",			unit_set_local_pose);

	env.load_module_function("Unit", "camera", unit_camera);
}

} // namespace crown