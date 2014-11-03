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

#include "lua_stack.h"
#include "lua_environment.h"
#include "unit.h"

namespace crown
{

static int unit_node(lua_State* L)
{
	LuaStack stack(L);
	stack.push_int32(stack.get_unit(1)->node(stack.get_string(2)));
	return 1;
}

static int unit_has_node(lua_State* L)
{
	LuaStack stack(L);
	stack.push_bool(stack.get_unit(1)->has_node(stack.get_string(2)));
	return 1;
}

static int unit_num_nodes(lua_State* L)
{
	LuaStack stack(L);
	stack.push_int32(stack.get_unit(1)->num_nodes());
	return 1;
}

static int unit_local_position(lua_State* L)
{
	LuaStack stack(L);
	stack.push_vector3(stack.get_unit(1)->local_position(stack.get_int(2)));
	return 1;
}

static int unit_local_rotation(lua_State* L)
{
	LuaStack stack(L);
	stack.push_quaternion(stack.get_unit(1)->local_rotation(stack.get_int(2)));
	return 1;
}

static int unit_local_pose(lua_State* L)
{
	LuaStack stack(L);
	stack.push_matrix4x4(stack.get_unit(1)->local_pose(stack.get_int(2)));
	return 1;
}

static int unit_world_position(lua_State* L)
{
	LuaStack stack(L);
	stack.push_vector3(stack.get_unit(1)->world_position(stack.get_int(2)));
	return 1;
}

static int unit_world_rotation(lua_State* L)
{
	LuaStack stack(L);
	stack.push_quaternion(stack.get_unit(1)->world_rotation(stack.get_int(2)));
	return 1;
}

static int unit_world_pose(lua_State* L)
{
	LuaStack stack(L);
	stack.push_matrix4x4(stack.get_unit(1)->world_pose(stack.get_int(2)));
	return 1;
}

static int unit_set_local_position(lua_State* L)
{
	LuaStack stack(L);
	stack.get_unit(1)->set_local_position(stack.get_int(2), stack.get_vector3(3));
	return 0;
}

static int unit_set_local_rotation(lua_State* L)
{
	LuaStack stack(L);
	stack.get_unit(1)->set_local_rotation(stack.get_int(2), stack.get_quaternion(3));
	return 0;
}

static int unit_set_local_pose(lua_State* L)
{
	LuaStack stack(L);
	stack.get_unit(1)->set_local_pose(stack.get_int(2), stack.get_matrix4x4(3));
	return 0;
}

static int unit_link_node(lua_State* L)
{
	LuaStack stack(L);
	stack.get_unit(1)->link_node(stack.get_int(2), stack.get_int(3));
	return 0;
}

static int unit_unlink_node(lua_State* L)
{
	LuaStack stack(L);
	stack.get_unit(1)->unlink_node(stack.get_int(2));
	return 0;
}

static int unit_camera(lua_State* L)
{
	LuaStack stack(L);

	Unit* unit = stack.get_unit(1);

	if (stack.is_number(2))
	{
		stack.push_camera(unit->camera((uint32_t) stack.get_int(2)));
		return 1;
	}

	stack.push_camera(unit->camera(stack.get_string(2)));
	return 1;
}

static int unit_material(lua_State* L)
{
	LuaStack stack(L);

	Unit* unit = stack.get_unit(1);

	if (stack.is_number(2))
	{
		stack.push_material(unit->material((uint32_t) stack.get_int(2)));
		return 1;
	}

	stack.push_material(unit->material(stack.get_string(2)));
	return 1;
}

static int unit_sprite(lua_State* L)
{
	LuaStack stack(L);

	Unit* unit = stack.get_unit(1);

	if (stack.is_number(2))
	{
		stack.push_sprite(unit->sprite((uint32_t) stack.get_int(2)));
		return 1;
	}

	stack.push_sprite(unit->sprite(stack.get_string(2)));
	return 1;
}

static int unit_actor(lua_State* L)
{
	LuaStack stack(L);

	Unit* unit = stack.get_unit(1);

	if (stack.is_number(2))
	{
		stack.push_actor(unit->actor((uint32_t) stack.get_int(2)));
		return 1;
	}

	stack.push_actor(unit->actor(stack.get_string(2)));
	return 1;
}

static int unit_controller(lua_State* L)
{
	LuaStack stack(L);
	Controller* ctl = stack.get_unit(1)->controller();
	ctl ? stack.push_controller(ctl) : stack.push_nil();
	return 1;
}

static int unit_is_a(lua_State* L)
{
	LuaStack stack(L);
	stack.push_bool(stack.get_unit(1)->is_a(stack.get_string(2)));
	return 1;
}

static int unit_play_sprite_animation(lua_State* L)
{
	LuaStack stack(L);
	stack.get_unit(1)->play_sprite_animation(stack.get_string(2), stack.get_bool(3));
	return 0;
}

static int unit_stop_sprite_animation(lua_State* L)
{
	LuaStack stack(L);
	stack.get_unit(1)->stop_sprite_animation();
	return 0;
}

static int unit_has_key(lua_State* L)
{
	LuaStack stack(L);
	stack.push_bool(stack.get_unit(1)->has_key(stack.get_string(2)));
	return 1;
}

static int unit_get_key(lua_State* L)
{
	LuaStack stack(L);

	Unit* unit = stack.get_unit(1);
	const char* key = stack.get_string(2);

	switch (unit->value_type(key))
	{
		case ValueType::BOOL:
		{
			bool val;
			unit->get_key(key, val);
			stack.push_bool(val);
			return 1;
		}
		case ValueType::FLOAT:
		{
			float val;
			unit->get_key(key, val);
			stack.push_float(val);
			return 1;
		}
		case ValueType::STRING:
		{
			StringId32 val;
			unit->get_key(key, val);
			stack.push_uint32(val);
			return 1;
		}
		case ValueType::VECTOR3:
		{
			Vector3 val;
			unit->get_key(key, val);
			stack.push_vector3(val);
			return 1;
		}
		default: CE_FATAL("Unknown value type"); break;
	}

	return 0;
}

static int unit_set_key(lua_State* L)
{
	LuaStack stack(L);

	Unit* unit = stack.get_unit(1);
	const char* key = stack.get_string(2);

	switch (stack.value_type(3))
	{
		case LUA_TBOOLEAN: unit->set_key(key, stack.get_bool(3)); break;
		case LUA_TNUMBER: unit->set_key(key, stack.get_float(3)); break;
		case LUA_TSTRING: unit->set_key(key, stack.get_string(3)); break;
		case LUA_TLIGHTUSERDATA: unit->set_key(key, stack.get_vector3(3)); break;
		default: CE_FATAL("Unsupported value type"); break; // FIXME use LUA_ASSERT
	}

	return 0;
}

void load_unit(LuaEnvironment& env)
{
	env.load_module_function("Unit", "node",                  unit_node);
	env.load_module_function("Unit", "has_node",              unit_has_node);
	env.load_module_function("Unit", "num_nodes",             unit_num_nodes);
	env.load_module_function("Unit", "local_position",        unit_local_position);
	env.load_module_function("Unit", "local_rotation",        unit_local_rotation);
	env.load_module_function("Unit", "local_pose",            unit_local_pose);
	env.load_module_function("Unit", "world_position",        unit_world_position);
	env.load_module_function("Unit", "world_rotation",        unit_world_rotation);
	env.load_module_function("Unit", "world_pose",            unit_world_pose);
	env.load_module_function("Unit", "set_local_position",    unit_set_local_position);
	env.load_module_function("Unit", "set_local_rotation",    unit_set_local_rotation);
	env.load_module_function("Unit", "set_local_pose",        unit_set_local_pose);
	env.load_module_function("Unit", "link_node",             unit_link_node);
	env.load_module_function("Unit", "unlink_node",           unit_unlink_node);
	env.load_module_function("Unit", "camera",                unit_camera);
	env.load_module_function("Unit", "material",              unit_material);
	env.load_module_function("Unit", "sprite",                unit_sprite);
	env.load_module_function("Unit", "actor",                 unit_actor);
	env.load_module_function("Unit", "controller",            unit_controller);
	env.load_module_function("Unit", "is_a",                  unit_is_a);
	env.load_module_function("Unit", "play_sprite_animation", unit_play_sprite_animation);
	env.load_module_function("Unit", "stop_sprite_animation", unit_stop_sprite_animation);
	env.load_module_function("Unit", "has_key",               unit_has_key);
	env.load_module_function("Unit", "get_key",               unit_get_key);
	env.load_module_function("Unit", "set_key",               unit_set_key);
}

} // namespace crown
