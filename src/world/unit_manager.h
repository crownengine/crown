/*
 * Copyright (c) 2012-2016 Daniele Bartolini and individual contributors.
 * License: https://github.com/taylor001/crown/blob/master/LICENSE
 */

#pragma once

#include "container_types.h"
#include "world_types.h"

namespace crown
{
// Reference: http://bitsquid.blogspot.it/2014/08/building-data-oriented-entity-system.html

/// Manages units.
///
/// @ingroup World
class UnitManager
{
	Array<uint8_t> _generation;
	Queue<uint32_t> _free_indices;

	typedef void (*DestroyFunction)(UnitId unit, void* user_ptr);

	struct DestroyData
	{
		DestroyFunction destroy;
		void* user_ptr;
	};

	Array<DestroyData> _destroy_callbacks;

public:

	UnitManager(Allocator& a);

	UnitId make_unit(uint32_t idx, uint8_t gen);

	/// Creates a new empty unit.
	UnitId create();

	/// Creates a new empty unit in the given @a world.
	UnitId create(World& world);

	/// Returns whether the unit @id is alive.
	bool alive(UnitId id) const;

	/// Destroys the unit @a id.
	void destroy(UnitId id);

	void register_destroy_function(DestroyFunction fn, void* user_ptr);

	void trigger_destroy_callbacks(UnitId id);
};

} // namespace crown
