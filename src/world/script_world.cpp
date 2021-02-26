/*
 * Copyright (c) 2012-2021 Daniele Bartolini et al.
 * License: https://github.com/dbartolini/crown/blob/master/LICENSE
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

namespace crown
{
namespace script_world_internal
{
	static ScriptInstance make_instance(u32 i)
	{
		ScriptInstance inst = { i };
		return inst;
	}

	static void unit_destroyed_callback(ScriptWorld& sw, UnitId unit, ScriptInstance i)
	{
		if (hash_map::has(sw._map, unit))
			script_world::destroy(sw, unit, i);
	}

	static void unit_destroyed_callback_bridge(UnitId unit, void* user_ptr)
	{
		unit_destroyed_callback(*((ScriptWorld*)user_ptr), unit, make_instance(UINT32_MAX));
	}
} // script_world_internal

namespace script_world
{
	ScriptInstance create(ScriptWorld& sw, UnitId unit, const ScriptDesc& desc)
	{
		CE_ASSERT(!hash_map::has(sw._map, unit), "Unit already has a script component");

		u32 script_i = hash_map::get(sw._cache
			, desc.script_resource
			, UINT32_MAX
			);

		ScriptWorld::ScriptData sd;
		sd.module_ref = LUA_REFNIL;

		if (script_i != UINT32_MAX)
		{
			sd = sw._script[script_i];
		}
		else
		{
			script_i = array::size(sw._script);

			sw._resource_manager->load(RESOURCE_TYPE_SCRIPT, desc.script_resource);
			sw._resource_manager->flush();
			const LuaResource* lr = (LuaResource*)sw._resource_manager->get(RESOURCE_TYPE_SCRIPT, desc.script_resource);

			LuaStack stack = sw._lua_environment->execute(lr, 1);
			stack.push_value(0);
			sd.module_ref = luaL_ref(stack.L, LUA_REGISTRYINDEX);
			stack.pop(1);

			array::push_back(sw._script, sd);
			hash_map::set(sw._cache, desc.script_resource, script_i);
		}

		ScriptWorld::InstanceData data;
		data.unit     = unit;
		data.script_i = script_i;

		u32 instance_i = array::size(sw._data);
		array::push_back(sw._data, data);
		hash_map::set(sw._map, unit, instance_i);

		if (!sw._disable_callbacks)
		{
			LuaStack stack(sw._lua_environment->L);
			lua_rawgeti(stack.L, LUA_REGISTRYINDEX, sd.module_ref);
			lua_getfield(stack.L, -1, "spawned");
			stack.push_world(sw._world);
			stack.push_table(1);
			stack.push_key_begin(1);
			stack.push_unit(unit);
			stack.push_key_end();
			int status = sw._lua_environment->call(2, 0);
			if (status != LUA_OK)
			{
				report(stack.L, status);
				device()->pause();
			}
			stack.pop(1);
		}

		return script_world_internal::make_instance(instance_i);
	}

	void destroy(ScriptWorld& sw, UnitId unit, ScriptInstance /*i*/)
	{
		CE_ASSERT(hash_map::has(sw._map, unit), "Unit does not have script component");

		const u32 unit_i    = hash_map::get(sw._map, unit, UINT32_MAX);
		const u32 last_i    = array::size(sw._data) - 1;
		const UnitId last_u = sw._data[last_i].unit;
		const u32 script_i  = sw._data[unit_i].script_i;

		if (!sw._disable_callbacks)
		{
			LuaStack stack(sw._lua_environment->L);
			lua_rawgeti(stack.L, LUA_REGISTRYINDEX, sw._script[script_i].module_ref);
			lua_getfield(stack.L, -1, "unspawned");
			stack.push_world(sw._world);
			stack.push_table(1);
			stack.push_key_begin(1);
			stack.push_unit(unit);
			stack.push_key_end();
			int status = sw._lua_environment->call(2, 0);
			if (status != LUA_OK)
			{
				report(stack.L, status);
				device()->pause();
			}
			stack.pop(1);
		}

		sw._data[unit_i] = sw._data[last_i];
		hash_map::set(sw._map, last_u, unit_i);
		array::pop_back(sw._data);
	}

	ScriptInstance instance(ScriptWorld& sw, UnitId unit)
	{
		return script_world_internal::make_instance(hash_map::get(sw._map, unit, UINT32_MAX));
	}

	void update(ScriptWorld& sw, f32 dt)
	{
		if (sw._disable_callbacks)
			return;

		LuaStack stack(sw._lua_environment->L);

		for (u32 i = 0; i < array::size(sw._script); ++i)
		{
			lua_rawgeti(stack.L, LUA_REGISTRYINDEX, sw._script[i].module_ref);
			lua_getfield(stack.L, -1, "update");
			stack.push_world(sw._world);
			stack.push_float(dt);
			int status = sw._lua_environment->call(2, 0);
			if (status != LUA_OK)
			{
				report(stack.L, status);
				device()->pause();
			}
			stack.pop(1);
		}
	}

	void collision(ScriptWorld& sw, const PhysicsCollisionEvent& ev)
	{
		if (sw._disable_callbacks)
			return;

		for (u32 i = 0; i < array::size(sw._data); ++i)
		{
			if (sw._data[i].unit == ev.units[0] || sw._data[i].unit == ev.units[1])
			{
				int unit_index = sw._data[i].unit == ev.units[0] ? 0 : 1;

				LuaStack stack(sw._lua_environment->L);
				lua_rawgeti(stack.L, LUA_REGISTRYINDEX, sw._script[sw._data[i].script_i].module_ref);
				switch (ev.type)
				{
				case PhysicsCollisionEvent::TOUCH_BEGIN:
					lua_getfield(stack.L, -1, "collision_begin");
					if (!lua_isnil(stack.L, -1))
					{
						stack.push_unit (ev.units[1-unit_index]);
						stack.push_unit (ev.units[unit_index]);
						stack.push_actor(ev.actors[unit_index]);
						stack.push_vector3(ev.position);
						stack.push_vector3(ev.normal);
						stack.push_float(ev.distance);
						int status = sw._lua_environment->call(6, 0);
						if (status != LUA_OK)
						{
							report(stack.L, status);
							device()->pause();
						}
						stack.pop(1);
					}
					break;

				case PhysicsCollisionEvent::TOUCHING:
					lua_getfield(stack.L, -1, "collision");
					if (!lua_isnil(stack.L, -1))
					{
						stack.push_unit (ev.units[1-unit_index]);
						stack.push_unit (ev.units[unit_index]);
						stack.push_actor(ev.actors[unit_index]);
						stack.push_vector3(ev.position);
						stack.push_vector3(ev.normal);
						stack.push_float(ev.distance);
						int status = sw._lua_environment->call(6, 0);
						if (status != LUA_OK)
						{
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

} // namespace script_world

ScriptWorld::ScriptWorld(Allocator& a, UnitManager& um, ResourceManager& rm, LuaEnvironment& le, World& w)
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
