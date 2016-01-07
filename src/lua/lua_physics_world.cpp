/*
 * Copyright (c) 2012-2016 Daniele Bartolini and individual contributors.
 * License: https://github.com/taylor001/crown/blob/master/LICENSE
 */

#include "lua_environment.h"
#include "lua_stack.h"
#include "physics_world.h"
#include "quaternion.h"
#include "memory.h"
#include "temp_allocator.h"

namespace crown
{

struct RaycastInfo
{
	const char* name;
	RaycastMode::Enum mode;
};

static RaycastInfo s_raycast[] =
{
	{ "closest", RaycastMode::CLOSEST },
	{ "all",     RaycastMode::ALL     }
};
CE_STATIC_ASSERT(CE_COUNTOF(s_raycast) == RaycastMode::COUNT);

static RaycastMode::Enum name_to_raycast_mode(const char* name)
{
	for (uint32_t i = 0; i < CE_COUNTOF(s_raycast); ++i)
	{
		if (strcmp(s_raycast[i].name, name) == 0)
			return s_raycast[i].mode;
	}

	return RaycastMode::COUNT;
}

static int physics_world_actor_instances(lua_State* L)
{
	LuaStack stack(L);
	ActorInstance inst = stack.get_physics_world(1)->actor(stack.get_unit(2));

	if (inst.i == UINT32_MAX)
		stack.push_nil();
	else
		stack.push_actor(inst);

	return 1;
}

static int physics_world_actor_world_position(lua_State* L)
{
	LuaStack stack(L);
	stack.push_vector3(stack.get_physics_world(1)->actor_world_position(stack.get_actor(2)));
	return 1;
}

static int physics_world_actor_world_rotation(lua_State* L)
{
	LuaStack stack(L);
	stack.push_quaternion(stack.get_physics_world(1)->actor_world_rotation(stack.get_actor(2)));
	return 1;
}

static int physics_world_actor_world_pose(lua_State* L)
{
	LuaStack stack(L);
	stack.push_matrix4x4(stack.get_physics_world(1)->actor_world_pose(stack.get_actor(2)));
	return 1;
}

static int physics_world_teleport_actor_world_position(lua_State* L)
{
	LuaStack stack(L);
	stack.get_physics_world(1)->teleport_actor_world_position(stack.get_actor(2), stack.get_vector3(3));
	return 0;
}

static int physics_world_teleport_actor_world_rotation(lua_State* L)
{
	LuaStack stack(L);
	stack.get_physics_world(1)->teleport_actor_world_rotation(stack.get_actor(2), stack.get_quaternion(3));
	return 0;
}

static int physics_world_teleport_actor_world_pose(lua_State* L)
{
	LuaStack stack(L);
	stack.get_physics_world(1)->teleport_actor_world_pose(stack.get_actor(2), stack.get_matrix4x4(3));
	return 0;
}

static int physics_world_actor_center_of_mass(lua_State* L)
{
	LuaStack stack(L);
	stack.push_vector3(stack.get_physics_world(1)->actor_center_of_mass(stack.get_actor(2)));
	return 1;
}

static int physics_world_enable_actor_gravity(lua_State* L)
{
	LuaStack stack(L);
	stack.get_physics_world(1)->enable_actor_gravity(stack.get_actor(2));
	return 0;
}

static int physics_world_disable_actor_gravity(lua_State* L)
{
	LuaStack stack(L);
	stack.get_physics_world(1)->disable_actor_gravity(stack.get_actor(2));
	return 0;
}

static int physics_world_enable_actor_collision(lua_State* L)
{
	LuaStack stack(L);
	stack.get_physics_world(1)->enable_actor_collision(stack.get_actor(2));
	return 0;
}

static int physics_world_disable_actor_collision(lua_State* L)
{
	LuaStack stack(L);
	stack.get_physics_world(1)->disable_actor_collision(stack.get_actor(2));
	return 0;
}

static int physics_world_set_actor_collision_filter(lua_State* L)
{
	LuaStack stack(L);
	stack.get_physics_world(1)->set_actor_collision_filter(stack.get_actor(2), stack.get_string_id(3));
	return 0;
}

static int physics_world_set_actor_kinematic(lua_State* L)
{
	LuaStack stack(L);
	stack.get_physics_world(1)->set_actor_kinematic(stack.get_actor(2), stack.get_bool(3));
	return 0;
}

static int physics_world_move_actor(lua_State* L)
{
	LuaStack stack(L);
	stack.get_physics_world(1)->move_actor(stack.get_actor(2), stack.get_vector3(3));
	return 0;
}

static int physics_world_is_static(lua_State* L)
{
	LuaStack stack(L);
	stack.push_bool(stack.get_physics_world(1)->is_static(stack.get_actor(2)));
	return 1;
}

static int physics_world_is_dynamic(lua_State* L)
{
	LuaStack stack(L);
	stack.push_bool(stack.get_physics_world(1)->is_dynamic(stack.get_actor(2)));
	return 1;
}

static int physics_world_is_kinematic(lua_State* L)
{
	LuaStack stack(L);
	stack.push_bool(stack.get_physics_world(1)->is_kinematic(stack.get_actor(2)));
	return 1;
}

static int physics_world_is_nonkinematic(lua_State* L)
{
	LuaStack stack(L);
	stack.push_bool(stack.get_physics_world(1)->is_nonkinematic(stack.get_actor(2)));
	return 1;
}


static int physics_world_actor_linear_damping(lua_State* L)
{
	LuaStack stack(L);
	stack.push_float(stack.get_physics_world(1)->actor_linear_damping(stack.get_actor(2)));
	return 1;
}

static int physics_world_set_actor_linear_damping(lua_State* L)
{
	LuaStack stack(L);
	stack.get_physics_world(1)->set_actor_linear_damping(stack.get_actor(2), stack.get_float(3));
	return 0;
}

static int physics_world_actor_angular_damping(lua_State* L)
{
	LuaStack stack(L);
	stack.push_float(stack.get_physics_world(1)->actor_angular_damping(stack.get_actor(2)));
	return 1;
}

static int physics_world_set_actor_angular_damping(lua_State* L)
{
	LuaStack stack(L);
	stack.get_physics_world(1)->set_actor_angular_damping(stack.get_actor(2), stack.get_float(3));
	return 0;
}

static int physics_world_actor_linear_velocity(lua_State* L)
{
	LuaStack stack(L);
	stack.push_vector3(stack.get_physics_world(1)->actor_linear_velocity(stack.get_actor(2)));
	return 1;
}

static int physics_world_set_actor_linear_velocity(lua_State* L)
{
	LuaStack stack(L);
	stack.get_physics_world(1)->set_actor_linear_velocity(stack.get_actor(2), stack.get_vector3(3));
	return 0;
}

static int physics_world_actor_angular_velocity(lua_State* L)
{
	LuaStack stack(L);
	stack.push_vector3(stack.get_physics_world(1)->actor_angular_velocity(stack.get_actor(2)));
	return 1;
}

static int physics_world_set_actor_angular_velocity(lua_State* L)
{
	LuaStack stack(L);
	stack.get_physics_world(1)->set_actor_angular_velocity(stack.get_actor(2), stack.get_vector3(3));
	return 0;
}

static int physics_world_add_actor_impulse(lua_State* L)
{
	LuaStack stack(L);
	stack.get_physics_world(1)->add_actor_impulse(stack.get_actor(2), stack.get_vector3(3));
	return 0;
}

static int physics_world_add_actor_impulse_at(lua_State* L)
{
	LuaStack stack(L);
	stack.get_physics_world(1)->add_actor_impulse_at(stack.get_actor(2), stack.get_vector3(3), stack.get_vector3(4));
	return 0;
}

static int physics_world_add_actor_torque_impulse(lua_State* L)
{
	LuaStack stack(L);
	stack.get_physics_world(1)->add_actor_torque_impulse(stack.get_actor(2), stack.get_vector3(3));
	return 0;
}

static int physics_world_push_actor(lua_State* L)
{
	LuaStack stack(L);
	stack.get_physics_world(1)->push_actor(stack.get_actor(2), stack.get_vector3(3), stack.get_float(4));
	return 0;
}

static int physics_world_push_actor_at(lua_State* L)
{
	LuaStack stack(L);
	stack.get_physics_world(1)->push_actor_at(stack.get_actor(2), stack.get_vector3(3), stack.get_float(4), stack.get_vector3(5));
	return 0;
}

static int physics_world_is_sleeping(lua_State* L)
{
	LuaStack stack(L);
	stack.push_bool(stack.get_physics_world(1)->is_sleeping(stack.get_actor(2)));
	return 1;
}

static int physics_world_wake_up(lua_State* L)
{
	LuaStack stack(L);
	stack.get_physics_world(1)->wake_up(stack.get_actor(2));
	return 0;
}

static int physics_world_controller_instances(lua_State* L)
{
	LuaStack stack(L);
	stack.push_controller(stack.get_physics_world(1)->controller(stack.get_unit(2)));
	return 1;
}

static int physics_world_move_controller(lua_State* L)
{
	LuaStack stack(L);
	stack.get_physics_world(1)->move_controller(stack.get_controller(2), stack.get_vector3(3));
	return 0;
}

static int physics_world_create_joint(lua_State* L)
{
	LuaStack stack(L);
	JointDesc jd;
	jd.type = JointType::SPRING;
	jd.anchor_0 = vector3(0, -2, 0);
	jd.anchor_1 = vector3(0, 2, 0);
	jd.break_force = 999999.0f;
	jd.hinge.axis = vector3(1, 0, 0);
	jd.hinge.lower_limit = -3.14f / 4.0f;
	jd.hinge.upper_limit = 3.14f / 4.0f;
	jd.hinge.bounciness = 12.0f;
	stack.get_physics_world(1)->create_joint(stack.get_actor(2), stack.get_actor(3), jd);
	return 0;
}

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

static int physics_world_raycast(lua_State* L)
{
	LuaStack stack(L);
	PhysicsWorld* world = stack.get_physics_world(1);

	TempAllocator1024 ta;
	Array<RaycastHit> hits(ta);

	world->raycast(stack.get_vector3(2)
		, stack.get_vector3(3)
		, stack.get_float(4)
		, name_to_raycast_mode(stack.get_string(5))
		, hits
		);

	stack.push_table();
	for (uint32_t i = 0; i < array::size(hits); ++i)
	{
		stack.push_key_begin(i+1);
		stack.push_actor(hits[i].actor);
		stack.push_key_end();
	}

	return 1;
}

static int physics_world_enable_debug_drawing(lua_State* L)
{
	LuaStack stack(L);
	stack.get_physics_world(1)->enable_debug_drawing(stack.get_bool(2));
	return 0;
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
	env.load_module_function("PhysicsWorld", "actor_instances",               physics_world_actor_instances);
	env.load_module_function("PhysicsWorld", "actor_world_position",          physics_world_actor_world_position);
	env.load_module_function("PhysicsWorld", "actor_world_rotation",          physics_world_actor_world_rotation);
	env.load_module_function("PhysicsWorld", "actor_world_pose",              physics_world_actor_world_pose);
	env.load_module_function("PhysicsWorld", "teleport_actor_world_position", physics_world_teleport_actor_world_position);
	env.load_module_function("PhysicsWorld", "teleport_actor_world_rotation", physics_world_teleport_actor_world_rotation);
	env.load_module_function("PhysicsWorld", "teleport_actor_world_pose",     physics_world_teleport_actor_world_pose);
	env.load_module_function("PhysicsWorld", "actor_center_of_mass",          physics_world_actor_center_of_mass);
	env.load_module_function("PhysicsWorld", "enable_actor_gravity",          physics_world_enable_actor_gravity);
	env.load_module_function("PhysicsWorld", "disable_actor_gravity",         physics_world_disable_actor_gravity);
	env.load_module_function("PhysicsWorld", "enable_actor_collision",        physics_world_enable_actor_collision);
	env.load_module_function("PhysicsWorld", "disable_actor_collision",       physics_world_disable_actor_collision);
	env.load_module_function("PhysicsWorld", "set_actor_collision_filter",    physics_world_set_actor_collision_filter);
	env.load_module_function("PhysicsWorld", "set_actor_kinematic",           physics_world_set_actor_kinematic);
	env.load_module_function("PhysicsWorld", "move_actor",                    physics_world_move_actor);
	env.load_module_function("PhysicsWorld", "is_static",                     physics_world_is_static);
	env.load_module_function("PhysicsWorld", "is_dynamic",                    physics_world_is_dynamic);
	env.load_module_function("PhysicsWorld", "is_kinematic",                  physics_world_is_kinematic);
	env.load_module_function("PhysicsWorld", "is_nonkinematic",               physics_world_is_nonkinematic);
	env.load_module_function("PhysicsWorld", "actor_linear_damping",          physics_world_actor_linear_damping);
	env.load_module_function("PhysicsWorld", "set_actor_linear_damping",      physics_world_set_actor_linear_damping);
	env.load_module_function("PhysicsWorld", "actor_angular_damping",         physics_world_actor_angular_damping);
	env.load_module_function("PhysicsWorld", "set_actor_angular_damping",     physics_world_set_actor_angular_damping);
	env.load_module_function("PhysicsWorld", "actor_linear_velocity",         physics_world_actor_linear_velocity);
	env.load_module_function("PhysicsWorld", "set_actor_linear_velocity",     physics_world_set_actor_linear_velocity);
	env.load_module_function("PhysicsWorld", "actor_angular_velocity",        physics_world_actor_angular_velocity);
	env.load_module_function("PhysicsWorld", "set_actor_angular_velocity",    physics_world_set_actor_angular_velocity);
	env.load_module_function("PhysicsWorld", "add_actor_impulse",             physics_world_add_actor_impulse);
	env.load_module_function("PhysicsWorld", "add_actor_impulse_at",          physics_world_add_actor_impulse_at);
	env.load_module_function("PhysicsWorld", "add_actor_torque_impulse",      physics_world_add_actor_torque_impulse);
	env.load_module_function("PhysicsWorld", "push_actor",                    physics_world_push_actor);
	env.load_module_function("PhysicsWorld", "push_actor_at",                 physics_world_push_actor_at);
	env.load_module_function("PhysicsWorld", "is_sleeping",                   physics_world_is_sleeping);
	env.load_module_function("PhysicsWorld", "wake_up",                       physics_world_wake_up);
	env.load_module_function("PhysicsWorld", "controller_instances",          physics_world_controller_instances);
	env.load_module_function("PhysicsWorld", "move_controller",               physics_world_move_controller);
	env.load_module_function("PhysicsWorld", "create_joint",                  physics_world_create_joint);
	env.load_module_function("PhysicsWorld", "gravity",                       physics_world_gravity);
	env.load_module_function("PhysicsWorld", "set_gravity",                   physics_world_set_gravity);
	env.load_module_function("PhysicsWorld", "raycast",                       physics_world_raycast);
	env.load_module_function("PhysicsWorld", "enable_debug_drawing",          physics_world_enable_debug_drawing);
	env.load_module_function("PhysicsWorld", "__index",                       "PhysicsWorld");
	env.load_module_function("PhysicsWorld", "__tostring",                    physics_world_tostring);
}

} // namespace crown
