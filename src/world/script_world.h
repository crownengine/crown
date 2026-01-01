/*
 * Copyright (c) 2012-2026 Daniele Bartolini et al.
 * SPDX-License-Identifier: MIT
 */

#include "core/containers/types.h"
#include "lua/types.h"
#include "resource/types.h"
#include "world/types.h"

namespace crown
{
/// Manages a set of scripts in a world.
///
/// @ingroup World
struct ScriptWorld
{
	struct ScriptData
	{
		int module_ref;
	};

	struct InstanceData
	{
		UnitId unit;
		u32 script_i;
	};

	u32 _marker;
	Array<ScriptData> _script;
	Array<InstanceData> _data;
	HashMap<UnitId, u32> _map;
	HashMap<StringId64, u32> _cache;

	UnitManager *_unit_manager;
	ResourceManager *_resource_manager;
	LuaEnvironment *_lua_environment;
	World *_world;
	UnitDestroyCallback _unit_destroy_callback;
	bool _disable_callbacks;

	///
	ScriptWorld(Allocator &a, UnitManager &um, ResourceManager &rm, LuaEnvironment &le, World &w);

	///
	~ScriptWorld();
};

namespace script_world
{
	///
	void create_instances(ScriptWorld &sw
		, const void *components_data
		, u32 num
		, const UnitId *unit_lookup
		, const u32 *unit_index
		);

	/// Creates a new component for the @a unit and returns its id.
	ScriptInstance create(ScriptWorld &sw, UnitId unit, const ScriptDesc &desc);

	/// Destroys the component for the @a unit.
	void destroy(ScriptWorld &sw, ScriptInstance i);

	/// Returns the component id for the @a unit.
	ScriptInstance instance(ScriptWorld &sw, UnitId unit);

	struct Index
	{
		int module_ref;
		u32 unit_index;
	};

	/// Calls @a function_name in all scripts if defined.
	void broadcast(ScriptWorld &sw
		, const char *function_name
		, const ArgType::Enum *arg_types = NULL
		, const Arg *args = NULL
		, u32 num_args = 0
		);

	///
	void multicast(ScriptWorld &sw
		, const char *function_name
		, const UnitId *units
		, const Index *index
		, u32 num_indices
		, const ArgType::Enum *arg_types = NULL
		, const Arg *args = NULL
		, u32 num_args = 0
		);

	///
	void unicast(ScriptWorld &sw
		, const char *function_name
		, ScriptInstance script_inst
		, const ArgType::Enum *arg_types = NULL
		, const Arg *args = NULL
		, u32 num_args = 0
		);

	///
	void spawned(ScriptWorld &sw, const UnitId *units, u32 num);

	///
	void unspawned(ScriptWorld &sw, const UnitId *units, u32 num);

} // namespace script_world

} // namespace crown
