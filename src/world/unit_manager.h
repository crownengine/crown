/*
 * Copyright (c) 2012-2018 Daniele Bartolini and individual contributors.
 * License: https://github.com/dbartolini/crown/blob/master/LICENSE
 */

#pragma once

#include "core/containers/types.h"
#include "world/types.h"

namespace crown
{
// Reference: http://bitsquid.blogspot.it/2014/08/building-data-oriented-entity-system.html

/// Manages units.
///
/// @ingroup World
struct UnitManager
{
	typedef void (*DestroyFunction)(UnitId unit, void* user_data);

	struct DestroyData
	{
		DestroyFunction destroy;
		void* user_data;
	};

	Array<u8> _generation;
	Queue<u32> _free_indices;
	Array<DestroyData> _destroy_callbacks;

	///
	UnitManager(Allocator& a);

	///
	UnitId make_unit(u32 idx, u8 gen);

	/// Creates a new empty unit.
	UnitId create();

	/// Creates a new empty unit in the given @a world.
	UnitId create(World& world);

	/// Returns whether the unit @id is alive.
	bool alive(UnitId id) const;

	/// Destroys the unit @a id.
	void destroy(UnitId id);

	void register_destroy_function(DestroyFunction fn, void* user_data);

	void unregister_destroy_function(void* user_data);

	void trigger_destroy_callbacks(UnitId id);
};

} // namespace crown
