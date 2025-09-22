/*
 * Copyright (c) 2012-2025 Daniele Bartolini et al.
 * SPDX-License-Identifier: MIT
 */

#include "core/containers/array.inl"
#include "core/containers/hash_map.inl"
#include "core/strings/string_id.inl"
#include "device/device.h"
#include "lua/lua_environment.h"
#include "lua/lua_stack.inl"
#include "resource/resource_manager.h"
#include "world/script_world.h"
#include "world/unit_manager.h"
#include <algorithm> // std::sort

namespace crown
{
namespace script_world_internal
{
	static ScriptInstance make_instance(u32 i)
	{
		ScriptInstance inst = { i };
		return inst;
	}

	static void unit_destroyed_callback(ScriptWorld &sw, UnitId unit)
	{
		ScriptInstance inst = script_world::instance(sw, unit);
		if (is_valid(inst))
			script_world::destroy(sw, inst);
	}

	static void unit_destroyed_callback_bridge(UnitId unit, void *user_ptr)
	{
		unit_destroyed_callback(*((ScriptWorld *)user_ptr), unit);
	}
} // script_world_internal

namespace script_world
{
	void create_instances(ScriptWorld &sw
		, const void *components_data
		, u32 num
		, const UnitId *unit_lookup
		, const u32 *unit_index
		)
	{
		const ScriptDesc *scripts = (ScriptDesc *)components_data;

		for (u32 i = 0; i < num; ++i) {
			UnitId unit = unit_lookup[unit_index[i]];
			CE_ASSERT(!hash_map::has(sw._map, unit), "Unit already has a script component");

			u32 script_i = hash_map::get(sw._cache
				, scripts[i].script_resource
				, UINT32_MAX
				);

			ScriptWorld::ScriptData sd;
			sd.module_ref = LUA_REFNIL;

			if (script_i != UINT32_MAX) {
				sd = sw._script[script_i];
			} else {
				script_i = array::size(sw._script);

				LuaStack stack = sw._lua_environment->require(scripts[i].script_resource_name, 1);
				stack.push_value(0);
				sd.module_ref = luaL_ref(stack.L, LUA_REGISTRYINDEX);
				stack.pop(1);

				array::push_back(sw._script, sd);
				hash_map::set(sw._cache, scripts[i].script_resource, script_i);
			}

			ScriptWorld::InstanceData data;
			data.unit     = unit;
			data.script_i = script_i;

			u32 instance_i = array::size(sw._data);
			array::push_back(sw._data, data);
			hash_map::set(sw._map, unit, instance_i);
		}
	}

	ScriptInstance create(ScriptWorld &sw, UnitId unit, const ScriptDesc &desc)
	{
		u32 unit_index = 0;
		create_instances(sw, &desc, 1, &unit, &unit_index);
		return instance(sw, unit);
	}

	void destroy(ScriptWorld &sw, ScriptInstance inst)
	{
		const u32 last_i  = array::size(sw._data) - 1;
		const UnitId unit = sw._data[inst.i].unit;
		const UnitId last = sw._data[last_i].unit;

		sw._data[inst.i] = sw._data[last_i];
		array::pop_back(sw._data);

		hash_map::set(sw._map, last, inst.i);
		hash_map::remove(sw._map, unit);
	}

	ScriptInstance instance(ScriptWorld &sw, UnitId unit)
	{
		return script_world_internal::make_instance(hash_map::get(sw._map, unit, UINT32_MAX));
	}

	void update(ScriptWorld &sw, f32 dt)
	{
		if (sw._disable_callbacks)
			return;

		LuaStack stack(sw._lua_environment->L);

		for (u32 i = 0; i < array::size(sw._script); ++i) {
			lua_rawgeti(stack.L, LUA_REGISTRYINDEX, sw._script[i].module_ref);
			lua_getfield(stack.L, -1, "update");
			stack.push_world(sw._world);
			stack.push_float(dt);
			int status = sw._lua_environment->call(2, 0);
			if (status != LUA_OK) {
				report(stack.L, status);
				device()->pause();
			}
			stack.pop(1);
		}
	}

	void collision(ScriptWorld &sw, const PhysicsCollisionEvent &ev)
	{
		if (sw._disable_callbacks)
			return;

		for (u32 i = 0; i < array::size(sw._data); ++i) {
			if (sw._data[i].unit == ev.units[0] || sw._data[i].unit == ev.units[1]) {
				int unit_index = sw._data[i].unit == ev.units[0] ? 0 : 1;

				LuaStack stack(sw._lua_environment->L);
				lua_rawgeti(stack.L, LUA_REGISTRYINDEX, sw._script[sw._data[i].script_i].module_ref);
				switch (ev.type) {
				case PhysicsCollisionEvent::TOUCH_BEGIN:
					lua_getfield(stack.L, -1, "collision_begin");
					if (!lua_isnil(stack.L, -1)) {
						stack.push_unit(ev.units[1 - unit_index]);
						stack.push_unit(ev.units[unit_index]);
						stack.push_actor(ev.actors[unit_index]);
						stack.push_vector3(ev.position);
						stack.push_vector3(ev.normal);
						stack.push_float(ev.distance);
						int status = sw._lua_environment->call(6, 0);
						if (status != LUA_OK) {
							report(stack.L, status);
							device()->pause();
						}
						stack.pop(1);
					}
					break;

				case PhysicsCollisionEvent::TOUCHING:
					lua_getfield(stack.L, -1, "collision");
					if (!lua_isnil(stack.L, -1)) {
						stack.push_unit(ev.units[1 - unit_index]);
						stack.push_unit(ev.units[unit_index]);
						stack.push_actor(ev.actors[unit_index]);
						stack.push_vector3(ev.position);
						stack.push_vector3(ev.normal);
						stack.push_float(ev.distance);
						int status = sw._lua_environment->call(6, 0);
						if (status != LUA_OK) {
							report(stack.L, status);
							device()->pause();
						}
						stack.pop(1);
					}
					break;

				case PhysicsCollisionEvent::TOUCH_END:
					lua_getfield(stack.L, -1, "collision_end");
					if (!lua_isnil(stack.L, -1)) {
						int status = sw._lua_environment->call(0, 0);
						if (status != LUA_OK) {
							report(stack.L, status);
							device()->pause();
						}
						stack.pop(1);
					}
					break;

				default:
					CE_FATAL("Unknown physics collision event");
					break;
				}
			}
		}

		// Unit not found
	}

	void multicast(ScriptWorld &sw
		, const char *function_name
		, const UnitId *units
		, const Index *index
		, u32 num_indices
		, const ArgType::Enum *arg_types
		, const Arg *args
		, u32 num_args
		)
	{
		LuaStack stack(sw._lua_environment->L);
		lua_rawgeti(stack.L, LUA_REGISTRYINDEX, index[0].module_ref);
		lua_getfield(stack.L, -1, function_name);

		stack.push_table(num_indices);
		for (u32 i = 0; i < num_indices; ++i) {
			stack.push_key_begin(1 + i);
			stack.push_unit(units[index[i].unit_index]);
			stack.push_key_end();
		}

		for (u32 i = 0; i < num_args; ++i) {
			switch (arg_types[i]) {
			case ArgType::NIL: stack.push_nil(); break;
			case ArgType::INT: stack.push_int(args[i].int_value); break;
			case ArgType::BOOL: stack.push_bool(args[i].bool_value); break;
			case ArgType::FLOAT: stack.push_float(args[i].float_value); break;
			case ArgType::STRING: stack.push_string(args[i].string_value); break;
			case ArgType::STRING_ID: stack.push_string_id(StringId32(args[i].string_id_value)); break;
			case ArgType::POINTER: stack.push_pointer((void *)args[i].pointer_value); break;
			case ArgType::FUNCTION: stack.push_function(args[i].cfunction_value); break;
			case ArgType::UNIT: stack.push_unit(args[i].unit_value); break;
			case ArgType::ID: stack.push_id(args[i].id_value); break;
			case ArgType::VECTOR3: stack.push_vector3(args[i].vector3_value); break;
			case ArgType::QUATERNION: stack.push_quaternion(args[i].quaternion_value); break;
			case ArgType::MATRIX4X4: stack.push_matrix4x4(args[i].matrix4x4_value); break;
			default: CE_FATAL("Unknown argument type"); break;
			}
		}

		int status = sw._lua_environment->call(1 + num_args, 0);
		if (status != LUA_OK) {
			report(stack.L, status);
			device()->pause();
		}
		stack.pop(1);
	}

	void units_with_script(ScriptWorld &sw, Array<Index> &index, const UnitId *units, u32 num)
	{
		for (u32 i = 0; i < num; ++i) {
			const u32 unit_i = hash_map::get(sw._map, units[i], UINT32_MAX);

			if (unit_i == UINT32_MAX)
				continue;

			array::push_back(index, { sw._script[sw._data[unit_i].script_i].module_ref, i });
		}
	}

	void multicast_group(ScriptWorld &sw
		, const char *function_name
		, const UnitId *units
		, u32 num
		, const ArgType::Enum *arg_types
		, const Arg *args
		, u32 num_args
		)
	{
		Array<Index> index(default_allocator());

		units_with_script(sw, index, units, num);

		// Sort index by module to call spawned in groups.
		std::sort(array::begin(index)
			, array::end(index)
			, [](const Index &a, const Index &b) {
				return a.module_ref < b.module_ref;
			});

		// Call function_name for each group of units sharing the same script.
		for (u32 i = 0, j; i < array::size(index); i = j) {
			for (j = i; j < array::size(index); ++j) {
				if (index[j].module_ref != index[i].module_ref)
					break;
			}

			script_world::multicast(sw
				, function_name
				, units
				, array::begin(index) + i
				, j - i
				, arg_types
				, args
				, num_args
				);
		}
	}

	void spawned(ScriptWorld &sw, const UnitId *units, u32 num)
	{
		if (sw._disable_callbacks)
			return;

		ArgType::Enum arg_types = ArgType::POINTER;
		Arg args;
		args.pointer_value = (void *)sw._world;
		script_world::multicast_group(sw, "spawned", units, num, &arg_types, &args, 1);
	}

	void unspawned(ScriptWorld &sw, const UnitId *units, u32 num)
	{
		if (sw._disable_callbacks)
			return;

		ArgType::Enum arg_types = ArgType::POINTER;
		Arg args;
		args.pointer_value = (void *)sw._world;
		script_world::multicast_group(sw, "unspawned", units, num, &arg_types, &args, 1);
	}

} // namespace script_world

ScriptWorld::ScriptWorld(Allocator &a, UnitManager &um, ResourceManager &rm, LuaEnvironment &le, World &w)
	: _marker(SCRIPT_WORLD_MARKER)
	, _script(a)
	, _data(a)
	, _map(a)
	, _cache(a)
	, _unit_manager(&um)
	, _resource_manager(&rm)
	, _lua_environment(&le)
	, _world(&w)
	, _disable_callbacks(false)
{
	_unit_destroy_callback.destroy = script_world_internal::unit_destroyed_callback_bridge;
	_unit_destroy_callback.user_data = this;
	_unit_destroy_callback.node.next = NULL;
	_unit_destroy_callback.node.prev = NULL;
	um.register_destroy_callback(&_unit_destroy_callback);
}

ScriptWorld::~ScriptWorld()
{
	_unit_manager->unregister_destroy_callback(&_unit_destroy_callback);
	_marker = 0;
}

} // namespace crown
