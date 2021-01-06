/*
 * Copyright (c) 2012-2021 Daniele Bartolini et al.
 * License: https://github.com/dbartolini/crown/blob/master/LICENSE
 */

#include "core/containers/array.inl"
#include "core/math/constants.h"
#include "core/strings/string_id.inl"
#include "resource/level_resource.h"
#include "resource/unit_resource.h"
#include "world/level.h"
#include "world/unit_manager.h"
#include "world/world.h"

namespace crown
{
Level::Level(Allocator& a, UnitManager& um, World& w, const LevelResource& lr)
	: _marker(LEVEL_MARKER)
	, _allocator(&a)
	, _unit_manager(&um)
	, _world(&w)
	, _resource(&lr)
	, _unit_lookup(a)
{
}

Level::~Level()
{
	_marker = 0;

	_node.next = NULL;
	_node.prev = NULL;
}

void Level::load(const Vector3& pos, const Quaternion& rot)
{
	// Spawn units
	const UnitResource* ur = level_resource::unit_resource(_resource);

	// Spawn units
	array::resize(_unit_lookup, ur->num_units);
	for (u32 i = 0; i < ur->num_units; ++i)
		_unit_lookup[i] = _unit_manager->create();

	spawn_units(*_world, ur, pos, rot, VECTOR3_ONE, array::begin(_unit_lookup));

	// Play sounds
	const u32 num_sounds = level_resource::num_sounds(_resource);
	for (u32 i = 0; i < num_sounds; ++i)
	{
		const LevelSound* ls = level_resource::get_sound(_resource, i);
		_world->play_sound(ls->name
			, ls->loop
			, ls->volume
			, ls->position
			, ls->range
			);
	}
}

UnitId Level::unit_by_name(StringId32 name)
{
	const StringId32* unit_names = level_resource::unit_names(_resource);
	for (u32 i = 0; i < _resource->num_units; ++i)
	{
		if (unit_names[i] == name)
			return _unit_lookup[i];
	}

	return UNIT_INVALID;
}

} // namespace crown
