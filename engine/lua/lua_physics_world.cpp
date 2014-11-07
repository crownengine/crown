/*
 * Copyright (c) 2012-2014 Daniele Bartolini and individual contributors.
 * License: https://github.com/taylor001/crown/blob/master/LICENSE
 */

#include "lua_environment.h"
#include "lua_stack.h"
#include "physics_world.h"
#include "quaternion.h"
#include "memory.h"

namespace crown
{

static int physics_world_gravity(lua_State* L)
{
	LuaStack stack(L);
	stack.push_vector3(stack.get_physics_world(1)->gravity());
	return 1;
}

static int physics_world_set_gravity(lua_State* L)
{
	LuaStack stack(L);
	stack.get_physics_world(1)->set_gravity(stack.get_vector3(2));
	return 0;
}

static int physics_world_make_raycast(lua_State* L)
{
	LuaStack stack(L);

	PhysicsWorld* world = stack.get_physics_world(1);
	/* const char* callback = */ stack.get_string(2);
	int mode = stack.get_int(3);
	int filter = stack.get_int(4);

	RaycastId raycast = world->create_raycast((CollisionMode::Enum) mode, (CollisionType::Enum) filter);

	stack.push_raycast(world->get_raycast(raycast));
	return 1;
}

static int physics_world_overlap_test(lua_State* L)
{
	LuaStack stack(L);

	PhysicsWorld* world = stack.get_physics_world(1);
	CollisionType::Enum filter = (CollisionType::Enum) stack.get_int(2);
	ShapeType::Enum shape_type = (ShapeType::Enum) stack.get_int(3);
	Vector3 pos = stack.get_vector3(4);
	Quaternion rot = stack.get_quaternion(5);
	Vector3 size = stack.get_vector3(6);


	Array<Actor*> actors(default_allocator());

	world->overlap_test(filter, shape_type, pos, rot, size, actors);

	stack.push_table();
	for (uint32_t i = 0; i < array::size(actors); i++)
	{
		stack.push_key_begin(i+1);
		stack.push_actor(actors[i]);
		stack.push_key_end();
	}

	return 1;
}

static int physics_world_tostring(lua_State* L)
{
	LuaStack stack(L);
	PhysicsWorld* pw = stack.get_physics_world(1);
	stack.push_fstring("PhysicsWorld (%p)", pw);
	return 1;
}

void load_physics_world(LuaEnvironment& env)
{
	env.load_module_function("PhysicsWorld", "gravity",      physics_world_gravity);
	env.load_module_function("PhysicsWorld", "set_gravity",  physics_world_set_gravity);
	env.load_module_function("PhysicsWorld", "make_raycast", physics_world_make_raycast);
	env.load_module_function("PhysicsWorld", "overlap_test", physics_world_overlap_test);
	env.load_module_function("PhysicsWorld", "__index",      "PhysicsWorld");
	env.load_module_function("PhysicsWorld", "__tostring",   physics_world_tostring);
	
	env.load_module_enum("ActorType", "STATIC",            ActorType::STATIC);
	env.load_module_enum("ActorType", "DYNAMIC_PHYSICAL",  ActorType::DYNAMIC_PHYSICAL);
	env.load_module_enum("ActorType", "DYNAMIC_KINEMATIC", ActorType::DYNAMIC_KINEMATIC);

	env.load_module_enum("ShapeType", "SPHERE",      ShapeType::SPHERE);
	env.load_module_enum("ShapeType", "CAPSULE",     ShapeType::CAPSULE);
	env.load_module_enum("ShapeType", "BOX",         ShapeType::BOX);
	env.load_module_enum("ShapeType", "PLANE",       ShapeType::PLANE);
	env.load_module_enum("ShapeType", "CONVEX_MESH", ShapeType::CONVEX_MESH);
	
	env.load_module_enum("CollisionMode", "CLOSEST", CollisionMode::CLOSEST);
	env.load_module_enum("CollisionMode", "ANY",     CollisionMode::ANY);
	env.load_module_enum("CollisionMode", "ALL",     CollisionMode::ALL);
	
	env.load_module_enum("CollisionType", "STATIC",  CollisionType::STATIC);
	env.load_module_enum("CollisionType", "DYNAMIC", CollisionType::DYNAMIC);
	env.load_module_enum("CollisionType", "BOTH",    CollisionType::BOTH);
}

} // namespace crown
