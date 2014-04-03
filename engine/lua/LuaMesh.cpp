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

#include "Mesh.h"
#include "Quaternion.h"
#include "LuaStack.h"
#include "LuaEnvironment.h"

namespace crown
{

//-----------------------------------------------------------------------------
static int mesh_local_position(lua_State* L)
{
	LuaStack stack(L);

	Mesh* mesh = stack.get_mesh(1);

	stack.push_vector3(mesh->local_position());
	return 1;
}

//-----------------------------------------------------------------------------
static int mesh_local_rotation(lua_State* L)
{
	LuaStack stack(L);

	Mesh* mesh = stack.get_mesh(1);

	stack.push_quaternion(mesh->local_rotation());
	return 1;
}

//-----------------------------------------------------------------------------
static int mesh_local_pose(lua_State* L)
{
	LuaStack stack(L);

	Mesh* mesh = stack.get_mesh(1);

	stack.push_matrix4x4(mesh->local_pose());
	return 1;
}

//-----------------------------------------------------------------------------
static int mesh_world_position(lua_State* L)
{
	LuaStack stack(L);

	Mesh* mesh = stack.get_mesh(1);

	stack.push_vector3(mesh->world_position());
	return 1;
}

//-----------------------------------------------------------------------------
static int mesh_world_rotation(lua_State* L)
{
	LuaStack stack(L);

	Mesh* mesh = stack.get_mesh(1);

	stack.push_quaternion(mesh->world_rotation());
	return 1;
}

//-----------------------------------------------------------------------------
static int mesh_world_pose(lua_State* L)
{
	LuaStack stack(L);

	Mesh* mesh = stack.get_mesh(1);

	stack.push_matrix4x4(mesh->world_pose());
	return 1;
}

//-----------------------------------------------------------------------------
static int mesh_set_local_position(lua_State* L)
{
	LuaStack stack(L);

	Mesh* mesh = stack.get_mesh(1);
	Unit* unit = stack.get_unit(2);
	const Vector3 pos = stack.get_vector3(3);

	mesh->set_local_position(unit, pos);
	return 0;
}

//-----------------------------------------------------------------------------
static int mesh_set_local_rotation(lua_State* L)
{
	LuaStack stack(L);

	Mesh* mesh = stack.get_mesh(1);
	Unit* unit = stack.get_unit(2);
	const Quaternion rot = stack.get_quaternion(3);

	mesh->set_local_rotation(unit, rot);
	return 0;
}

//-----------------------------------------------------------------------------
static int mesh_set_local_pose(lua_State* L)
{
	LuaStack stack(L);

	Mesh* mesh = stack.get_mesh(1);
	Unit* unit = stack.get_unit(2);
	const Matrix4x4 pose = stack.get_matrix4x4(3);

	mesh->set_local_pose(unit, pose);
	return 0;
}

//-----------------------------------------------------------------------------
void load_mesh(LuaEnvironment& env)
{
	env.load_module_function("Mesh", "local_position",         mesh_local_position);
	env.load_module_function("Mesh", "local_rotation",         mesh_local_rotation);
	env.load_module_function("Mesh", "local_pose",             mesh_local_pose);
	env.load_module_function("Mesh", "world_position",         mesh_world_position);
	env.load_module_function("Mesh", "world_rotation",         mesh_world_rotation);
	env.load_module_function("Mesh", "world_pose",             mesh_world_pose);
	env.load_module_function("Mesh", "set_local_position",     mesh_set_local_position);
	env.load_module_function("Mesh", "set_local_rotation",     mesh_set_local_rotation);
	env.load_module_function("Mesh", "set_local_pose",         mesh_set_local_pose);
}

} // namespace crown
