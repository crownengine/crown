/*
 * Copyright (c) 2012-2016 Daniele Bartolini and individual contributors.
 * License: https://github.com/taylor001/crown/blob/master/LICENSE
 */

#pragma once

#include "world_types.h"
#include "memory_types.h"
#include "resource_types.h"
#include "math_types.h"

namespace crown
{

/// Game level.
///
/// @ingroup World
class Level
{
public:

	Level(Allocator& a, World& w, const LevelResource& lr);
	~Level();

	void load(const Vector3& pos, const Quaternion& rot);

public:

	enum { MARKER = 0x1f2b43fe };

private:

	uint32_t _marker;

	Allocator* _allocator;
	World* _world;
	const LevelResource* _resource;
};

} // namespace crown
