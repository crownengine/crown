/*
 * Copyright (c) 2012-2016 Daniele Bartolini and individual contributors.
 * License: https://github.com/taylor001/crown/blob/master/LICENSE
 */

#include "level.h"
#include "level_resource.h"
#include "world.h"

namespace crown
{

Level::Level(Allocator& a, World& w, const LevelResource& lr)
	: _marker(MARKER)
	, _allocator(&a)
	, _world(&w)
	, _resource(&lr)
{
}

Level::~Level()
{
	_marker = 0;
}

void Level::load(const Vector3& pos, const Quaternion& rot)
{
	_world->spawn_unit(*level_resource::get_units(_resource), pos, rot);
}

} // namespace crown
