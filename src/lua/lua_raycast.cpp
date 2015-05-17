/*
 * Copyright (c) 2012-2015 Daniele Bartolini and individual contributors.
 * License: https://github.com/taylor001/crown/blob/master/LICENSE
 */

#include "lua_stack.h"
#include "lua_environment.h"
#include "raycast.h"
#include "memory.h"
#include "array.h"

namespace crown
{

static int raycast_cast(lua_State* L)
{
	LuaStack stack(L);

	Raycast* raycast = stack.get_raycast(1);
	Vector3 from = stack.get_vector3(2);
	Vector3 dir = stack.get_vector3(3);
	float length = stack.get_float(4);

	Array<RaycastHit> hits(default_allocator());

	raycast->cast(from, dir, length, hits);

	switch(raycast->mode())
	{
		case CollisionMode::CLOSEST:
		{
			bool hit = array::size(hits) > 0 ? true : false;
			stack.push_bool(hit);
			break;
		}
		case CollisionMode::ANY:
		{
			bool hit = array::size(hits) > 0 ? true : false;
			stack.push_bool(hit);
			if (hit)
			{
				stack.push_vector3(hits[0].position);
				stack.push_float(hits[0].distance);
				stack.push_vector3(hits[0].normal);
				stack.push_actor(hits[0].actor);
				return 5;
			}
			break;
		}
		case CollisionMode::ALL:
		{
			stack.push_table();
			for (uint32_t i = 0; i < array::size(hits); i++)
			{
				stack.push_key_begin(i+1);

				stack.push_table();
				stack.push_key_begin("position"); stack.push_vector3(hits[i].position);	stack.push_key_end();
				stack.push_key_begin("distance"); stack.push_float(hits[i].distance); stack.push_key_end();
				stack.push_key_begin("normal"); stack.push_vector3(hits[i].normal); stack.push_key_end();
				stack.push_key_begin("actor"); stack.push_actor(hits[i].actor); stack.push_key_end();

				stack.push_key_end();
			}
			break;
		}
	}

	return 1;
}

void load_raycast(LuaEnvironment& env)
{
	env.load_module_function("Raycast", "cast", raycast_cast);
}

} // namespace crown
