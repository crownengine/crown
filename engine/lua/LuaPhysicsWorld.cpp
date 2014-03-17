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

#include "LuaEnvironment.h"
#include "LuaStack.h"
#include "PhysicsWorld.h"
#include "Quaternion.h"

namespace crown
{

//-----------------------------------------------------------------------------
CE_EXPORT int physics_world_gravity(lua_State* L)
{
	LuaStack stack(L);

	PhysicsWorld* world = stack.get_physics_world(1);

	stack.push_vector3(world->gravity());
	return 1;
}

//-----------------------------------------------------------------------------
CE_EXPORT int physics_world_set_gravity(lua_State* L)
{
	LuaStack stack(L);

	PhysicsWorld* world = stack.get_physics_world(1);
	const Vector3& gravity = stack.get_vector3(2);

	world->set_gravity(gravity);
	return 0;
}

//-----------------------------------------------------------------------------
CE_EXPORT int physics_world_make_raycast(lua_State* L)
{
	LuaStack stack(L);

	PhysicsWorld* world = stack.get_physics_world(1);
	const char* callback = stack.get_string(2);
	int mode = stack.get_int(3);
	int filter = stack.get_int(4);

	RaycastId raycast = world->create_raycast(callback, (SceneQueryMode::Enum) mode, (SceneQueryFilter::Enum) filter);

	stack.push_raycast(world->lookup_raycast(raycast));
	return 1;
}

//-----------------------------------------------------------------------------
CE_EXPORT int physics_world_overlap_test(lua_State* L)
{
	LuaStack stack(L);

	PhysicsWorld* world = stack.get_physics_world(1);
	const char* callback = stack.get_string(2);
	int mode = stack.get_int(3);
	int filter = stack.get_int(4);
	int shape_type = stack.get_int(5);
	Vector3 pos = stack.get_vector3(6);
	Quaternion rot = stack.get_quaternion(7);
	Vector3 size = stack.get_vector3(8);

	world->overlap_test(callback, (SceneQueryMode::Enum) mode, (SceneQueryFilter::Enum) filter,
						(ShapeType::Enum) shape_type, pos, rot, size);

	return 0;
}

//-----------------------------------------------------------------------------
CE_EXPORT int physics_world_sync_overlap_test(lua_State* L)
{
	LuaStack stack(L);

	PhysicsWorld* world = stack.get_physics_world(1);
	const char* callback = stack.get_string(2);
	int mode = stack.get_int(3);
	int filter = stack.get_int(4);
	int shape_type = stack.get_int(5);
	Vector3 pos = stack.get_vector3(6);
	Quaternion rot = stack.get_quaternion(7);
	Vector3 size = stack.get_vector3(8);

	Actor* actor = world->sync_overlap_test(callback, (SceneQueryMode::Enum) mode, (SceneQueryFilter::Enum) filter,
										(ShapeType::Enum) shape_type, pos, rot, size);

	if (actor)
		stack.push_actor(actor);
	else
		stack.push_nil();
	
	return 1;
}

//-----------------------------------------------------------------------------
void load_physics_world(LuaEnvironment& env)
{
	env.load_module_function("PhysicsWorld", "gravity",				physics_world_gravity);
	env.load_module_function("PhysicsWorld", "set_gravity",			physics_world_set_gravity);
	env.load_module_function("PhysicsWorld", "make_raycast",		physics_world_make_raycast);
	env.load_module_function("PhysicsWorld", "overlap_test",		physics_world_overlap_test);
	env.load_module_function("PhysicsWorld", "sync_overlap_test",	physics_world_sync_overlap_test);

	// Actor types
	env.load_module_enum("ActorType", "STATIC",						ActorType::STATIC);
	env.load_module_enum("ActorType", "DYNAMIC_PHYSICAL",			ActorType::DYNAMIC_PHYSICAL);
	env.load_module_enum("ActorType", "DYNAMIC_KINEMATIC",			ActorType::DYNAMIC_KINEMATIC);

	// Shape types
	env.load_module_enum("ShapeType", "SPHERE",						ShapeType::SPHERE);
	env.load_module_enum("ShapeType", "CAPSULE",					ShapeType::CAPSULE);
	env.load_module_enum("ShapeType", "BOX",						ShapeType::BOX);
	env.load_module_enum("ShapeType", "PLANE",						ShapeType::PLANE);
	env.load_module_enum("ShapeType", "CONVEX_MESH",				ShapeType::CONVEX_MESH);

	// SceneQuery types
	env.load_module_enum("SceneQueryType", "RAYCAST",				SceneQueryType::RAYCAST);
	env.load_module_enum("SceneQueryType", "OVERLAP",				SceneQueryType::OVERLAP);

	// SceneQuery modes
	env.load_module_enum("SceneQueryMode", "CLOSEST",				SceneQueryMode::CLOSEST);
	env.load_module_enum("SceneQueryMode", "ANY",					SceneQueryMode::ANY);
	env.load_module_enum("SceneQueryMode", "ALL",					SceneQueryMode::ALL);

	// SceneQuery filters
	env.load_module_enum("SceneQueryFilter", "STATIC",				SceneQueryFilter::STATIC);
	env.load_module_enum("SceneQueryFilter", "DYNAMIC",				SceneQueryFilter::DYNAMIC);
	env.load_module_enum("SceneQueryFilter", "BOTH",				SceneQueryFilter::BOTH);
}

} // namespace crown