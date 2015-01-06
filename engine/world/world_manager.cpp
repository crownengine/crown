/*
 * Copyright (c) 2012-2015 Daniele Bartolini and individual contributors.
 * License: https://github.com/taylor001/crown/blob/master/LICENSE
 */

#include "world_manager.h"
#include "world.h"
#include "memory.h"

namespace crown
{

WorldManager::WorldManager()
	: m_allocator("world-manager", default_allocator())
{
}

WorldManager::~WorldManager()
{
}

WorldId WorldManager::create_world()
{
	World* w = CE_NEW(m_allocator, World)();
	const WorldId w_id = id_array::create(m_worlds, w);
	w->set_id(w_id);
	return w_id;
}

void WorldManager::destroy_world(WorldId id)
{
	CE_DELETE(m_allocator, id_array::get(m_worlds, id));
	id_array::destroy(m_worlds, id);
}

World* WorldManager::lookup_world(WorldId id)
{
	return id_array::get(m_worlds, id);
}

IdArray<CE_MAX_WORLDS, World*>& WorldManager::worlds()
{
	return m_worlds;
}

} // namespace crown
