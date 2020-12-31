/*
 * Copyright (c) 2012-2021 Daniele Bartolini et al.
 * License: https://github.com/dbartolini/crown/blob/master/LICENSE
 */

#include "core/containers/array.inl"
#include "core/containers/queue.inl"
#include "core/list.inl"
#include "world/unit_manager.h"
#include "world/world.h"

#define MINIMUM_FREE_INDICES 1024

namespace crown
{
UnitManager::UnitManager(Allocator& a)
	: _generation(a)
	, _free_indices(a)
{
	list::init_head(_callbacks.node);
}

UnitId UnitManager::make_unit(u32 idx, u8 gen)
{
	UnitId unit = { 0 | idx | u32(gen) << UNIT_INDEX_BITS };
	return unit;
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

bool UnitManager::alive(UnitId unit) const
{
	return _generation[unit.index()] == unit.id();
}

void UnitManager::destroy(UnitId unit)
{
	const u32 idx = unit.index();
	++_generation[idx];
	queue::push_back(_free_indices, idx);

	trigger_destroy_callbacks(unit);
}

void UnitManager::register_destroy_callback(UnitDestroyCallback* udc)
{
	list::add(udc->node, _callbacks.node);
}

void UnitManager::unregister_destroy_callback(UnitDestroyCallback* udc)
{
	list::remove(udc->node);
}

void UnitManager::trigger_destroy_callbacks(UnitId unit)
{
	ListNode* cur;
	list_for_each(cur, &_callbacks.node)
	{
		UnitDestroyCallback* udc = (UnitDestroyCallback*)container_of(cur, UnitDestroyCallback, node);
		udc->destroy(unit, udc->user_data);
	}
}

} // namespace crown
