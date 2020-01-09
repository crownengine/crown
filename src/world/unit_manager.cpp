/*
 * Copyright (c) 2012-2020 Daniele Bartolini and individual contributors.
 * License: https://github.com/dbartolini/crown/blob/master/LICENSE
 */

#include "core/containers/array.h"
#include "core/containers/queue.h"
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

void UnitManager::register_destroy_callback(UnitDestroyCallback* udc)
{
	list::add(udc->node, _callbacks.node);
}

void UnitManager::unregister_destroy_callback(UnitDestroyCallback* udc)
{
	list::remove(udc->node);
}

void UnitManager::trigger_destroy_callbacks(UnitId id)
{
	ListNode* cur;
	list_for_each(cur, &_callbacks.node)
	{
		UnitDestroyCallback* udc = (UnitDestroyCallback*)container_of(cur, UnitDestroyCallback, node);
		udc->destroy(id, udc->user_data);
	}
}

} // namespace crown
