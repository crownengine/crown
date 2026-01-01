/*
 * Copyright (c) 2012-2026 Daniele Bartolini et al.
 * SPDX-License-Identifier: MIT
 */

#include "core/list.inl"
#include "core/math/constants.h"
#include "core/memory/memory.inl"
#include "core/strings/string_id.inl"
#include "resource/level_resource.h"
#include "world/level.h"

namespace crown
{
Level::Level(const LevelResource *level_resource, UnitId *unit_lookup)
	: _marker(LEVEL_MARKER)
	, _resource(level_resource)
	, _unit_lookup(unit_lookup)
{
	list::init_head(_node);
}

Level::~Level()
{
	_marker = 0;

	_node.next = NULL;
	_node.prev = NULL;
}

UnitId Level::unit_by_name(StringId32 name)
{
	const StringId32 *unit_names = level_resource::unit_names(_resource);
	for (u32 i = 0; i < _resource->num_units; ++i) {
		if (unit_names[i] == name)
			return _unit_lookup[i];
	}

	return UNIT_INVALID;
}

namespace level
{
	Level *create(Allocator &a, const LevelResource *level_resource)
	{
		Level *l = (Level *)a.allocate(sizeof(*l) + level_resource->num_units*sizeof(UnitId));
		return new (l) Level(level_resource, (UnitId *)&l[1]);
	}

	void destroy(Allocator &a, Level *level)
	{
		CE_DELETE(a, level);
	}

} // namespace level

} // namespace crown
