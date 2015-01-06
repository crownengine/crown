/*
 * Copyright (c) 2012-2015 Daniele Bartolini and individual contributors.
 * License: https://github.com/taylor001/crown/blob/master/LICENSE
 */

#include "id_array.h"
#include "proxy_allocator.h"

#pragma once

namespace crown
{

typedef Id WorldId;
class World;

/// Manages a number of worlds.
///
/// @ingroup World
class WorldManager
{
public:

					WorldManager();
					~WorldManager();

	WorldId			create_world();
	void			destroy_world(WorldId id);

	World*			lookup_world(WorldId id);

	/// Return the array of worlds.
	IdArray<CE_MAX_WORLDS, World*>& worlds();

private:

	ProxyAllocator m_allocator;
	IdArray<CE_MAX_WORLDS, World*> m_worlds;
};

} // namespace crown
