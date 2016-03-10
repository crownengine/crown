/*
 * Copyright (c) 2012-2016 Daniele Bartolini and individual contributors.
 * License: https://github.com/taylor001/crown/blob/master/LICENSE
 */

#include "unit_manager.h"
#include "array.h"
#include "queue.h"
#include "world.h"

#define MINIMUM_FREE_INDICES 1024

namespace crown
{
UnitManager::UnitManager(Allocator& a)
	: _generation(a)
	, _free_indices(a)
	, _destroy_callbacks(a)
{
}

UnitId UnitManager::make_unit(u32 idx, u8 gen)
{
	UnitId id = { 0 | idx | u32(gen) << UNIT_INDEX_BITS };
	return id;
}

UnitId UnitManager::create()
{
	u32 idx;

	if (queue::size(_free_indices) > MINIMUM_FREE_INDICES)
	{
		idx = queue::front(_free_indices);
		queue::pop_front(_free_indices);
	}
	else
	{
		array::push_back(_generation, u8(0));
		idx = array::size(_generation) - 1;
		CE_ASSERT(idx < (1 << UNIT_INDEX_BITS), "Indices out of bounds");
	}

	return make_unit(idx, _generation[idx]);
}

UnitId UnitManager::create(World& world)
{
	return world.spawn_empty_unit();
}

bool UnitManager::alive(UnitId id) const
{
	return _generation[id.index()] == id.id();
}

void UnitManager::destroy(UnitId id)
{
	const u32 idx = id.index();
	++_generation[idx];
	queue::push_back(_free_indices, idx);

	trigger_destroy_callbacks(id);
}

void UnitManager::register_destroy_function(DestroyFunction fn, void* user_ptr)
{
	DestroyData dd;
	dd.destroy = fn;
	dd.user_ptr = user_ptr;
	array::push_back(_destroy_callbacks, dd);
}

void UnitManager::unregister_destroy_function(void* user_ptr)
{
	for (u32 i = 0, n = array::size(_destroy_callbacks); i < n; ++i)
	{
		if (_destroy_callbacks[i].user_ptr == user_ptr)
		{
			_destroy_callbacks[i] = _destroy_callbacks[n - 1];
			array::pop_back(_destroy_callbacks);
			return;
		}
	}

	CE_ASSERT(false, "Bad destroy function");
}

void UnitManager::trigger_destroy_callbacks(UnitId id)
{
	for (u32 i = 0; i < array::size(_destroy_callbacks); ++i)
		_destroy_callbacks[i].destroy(id, _destroy_callbacks[i].user_ptr);
}

} // namespace crown
