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
CE_EXPORT int unit_node(lua_State* L)
{
	LuaStack stack(L);

	Unit* unit = stack.get_unit(1);
	const char* name = stack.get_string(2);

	stack.push_int32(unit->node(name));
	return 1;
}

//-----------------------------------------------------------------------------
CE_EXPORT int unit_has_node(lua_State* L)
{
	LuaStack stack(L);

	Unit* unit = stack.get_unit(1);
	const char* name = stack.get_string(2);

	stack.push_bool(unit->has_node(name));
	return 1;
}

//-----------------------------------------------------------------------------
CE_EXPORT int unit_num_nodes(lua_State* L)
{
	LuaStack stack(L);

	Unit* unit = stack.get_unit(1);

	stack.push_int32(unit->num_nodes());
	return 1;
}

//-----------------------------------------------------------------------------
CE_EXPORT int unit_local_position(lua_State* L)
{
	LuaStack stack(L);

	Unit* unit = stack.get_unit(1);
	const int32_t node = stack.get_int(2);

	stack.push_vector3(unit->local_position(node));
	return 1;
}

//-----------------------------------------------------------------------------
CE_EXPORT int unit_local_rotation(lua_State* L)
{
	LuaStack stack(L);

	Unit* unit = stack.get_unit(1);
	const int32_t node = stack.get_int(2);

	stack.push_quaternion(unit->local_rotation(node));
	return 1;
}

//-----------------------------------------------------------------------------
CE_EXPORT int unit_local_pose(lua_State* L)
{
	LuaStack stack(L);

	Unit* unit = stack.get_unit(1);
	const int32_t node = stack.get_int(2);

	stack.push_matrix4x4(unit->local_pose(node));
	return 1;
}

//-----------------------------------------------------------------------------
CE_EXPORT int unit_world_position(lua_State* L)
{
	LuaStack stack(L);

	Unit* unit = stack.get_unit(1);
	const int32_t node = stack.get_int(2);

	stack.push_vector3(unit->world_position(node));
	return 1;
}

//-----------------------------------------------------------------------------
CE_EXPORT int unit_world_rotation(lua_State* L)
{
	LuaStack stack(L);

	Unit* unit = stack.get_unit(1);
	const int32_t node = stack.get_int(2);

	stack.push_quaternion(unit->world_rotation(node));
	return 1;
}

//-----------------------------------------------------------------------------
CE_EXPORT int unit_world_pose(lua_State* L)
{
	LuaStack stack(L);

	Unit* unit = stack.get_unit(1);
	const int32_t node = stack.get_int(2);

	stack.push_matrix4x4(unit->world_pose(node));
	return 1;
}

//-----------------------------------------------------------------------------
CE_EXPORT int unit_set_local_position(lua_State* L)
{
	LuaStack stack(L);

	Unit* unit = stack.get_unit(1);
	const int32_t node = stack.get_int(2);
	Vector3& pos = stack.get_vector3(3);

	unit->set_local_position(node, pos);
	return 0;
}

//-----------------------------------------------------------------------------
CE_EXPORT int unit_set_local_rotation(lua_State* L)
{
	LuaStack stack(L);

	Unit* unit = stack.get_unit(1);
	const int32_t node = stack.get_int(2);
	Quaternion& rot = stack.get_quaternion(3);

	unit->set_local_rotation(node, rot);
	return 0;
}

//-----------------------------------------------------------------------------
CE_EXPORT int unit_set_local_pose(lua_State* L)
{
	LuaStack stack(L);

	Unit* unit = stack.get_unit(1);
	const int32_t node = stack.get_int(2);
	Matrix4x4& pose = stack.get_matrix4x4(3);

	unit->set_local_pose(node, pose);
	return 0;
}

//-----------------------------------------------------------------------------
CE_EXPORT int unit_link_node(lua_State* L)
{
	LuaStack stack(L);

	Unit* unit = stack.get_unit(1);
	const int32_t child = stack.get_int(2);
	const int32_t parent = stack.get_int(3);

	unit->link_node(child, parent);
	return 0;
}

//-----------------------------------------------------------------------------
CE_EXPORT int unit_unlink_node(lua_State* L)
{
	LuaStack stack(L);

	Unit* unit = stack.get_unit(1);
	const int32_t child = stack.get_int(2);

	unit->unlink_node(child);
	return 0;
}

//-----------------------------------------------------------------------------
CE_EXPORT int unit_camera(lua_State* L)
{
	LuaStack stack(L);

	Unit* unit = stack.get_unit(1);
	const char* camera_name = stack.get_string(2);

	stack.push_camera(unit->camera(camera_name));
	return 1;
}

//-----------------------------------------------------------------------------
CE_EXPORT int unit_mesh(lua_State* L)
{
	LuaStack stack(L);

	Unit* unit = stack.get_unit(1);
	const char* mesh_name = stack.get_string(2);

	stack.push_mesh(unit->mesh(mesh_name));
	return 1;
}

//-----------------------------------------------------------------------------
CE_EXPORT int unit_sprite(lua_State* L)
{
	LuaStack stack(L);

	Unit* unit = stack.get_unit(1);
	const char* sprite_name = stack.get_string(2);

	stack.push_sprite(unit->sprite(sprite_name));
	return 1;
}

//-----------------------------------------------------------------------------
CE_EXPORT int unit_actor(lua_State* L)
{
	LuaStack stack(L);

	Unit* unit = stack.get_unit(1);
	const char* actor_name = stack.get_string(2);

	stack.push_actor(unit->actor(actor_name));
	return 1;
}

//-----------------------------------------------------------------------------
CE_EXPORT int unit_controller(lua_State* L)
{
	LuaStack stack(L);

	Unit* unit = stack.get_unit(1);

	Controller* controller = unit->controller();
	controller ? stack.push_controller(controller) : stack.push_nil();
	return 1;
}

//-----------------------------------------------------------------------------
CE_EXPORT int unit_is_a(lua_State* L)
{
	LuaStack stack(L);

	Unit* unit = stack.get_unit(1);
	const char* name = stack.get_string(2);

	stack.push_bool(unit->is_a(name));
	return 1;
}

//-----------------------------------------------------------------------------
void load_unit(LuaEnvironment& env)
{
	env.load_module_function("Unit", "node",					unit_node);
	env.load_module_function("Unit", "has_node",				unit_has_node);
	env.load_module_function("Unit", "num_nodes",				unit_num_nodes);

	env.load_module_function("Unit", "local_position",			unit_local_position);
	env.load_module_function("Unit", "local_rotation",			unit_local_rotation);
	env.load_module_function("Unit", "local_pose",				unit_local_pose);
	env.load_module_function("Unit", "world_position",			unit_world_position);
	env.load_module_function("Unit", "world_rotation",			unit_world_rotation);
	env.load_module_function("Unit", "world_pose",				unit_world_pose);
	env.load_module_function("Unit", "set_local_position",		unit_set_local_position);
	env.load_module_function("Unit", "set_local_rotation",		unit_set_local_rotation);
	env.load_module_function("Unit", "set_local_pose",			unit_set_local_pose);
	env.load_module_function("Unit", "link_node",				unit_link_node);
	env.load_module_function("Unit", "unlink_node",				unit_unlink_node);

	env.load_module_function("Unit", "camera",					unit_camera);
	env.load_module_function("Unit", "mesh",					unit_mesh);	
	env.load_module_function("Unit", "sprite",					unit_sprite);
	env.load_module_function("Unit", "actor",					unit_actor);
	env.load_module_function("Unit", "controller",				unit_controller);

	env.load_module_function("Unit", "is_a",					unit_is_a);
}

} // namespace crown
