/*
 * Copyright (c) 2012-2026 Daniele Bartolini et al.
 * SPDX-License-Identifier: MIT
 */

#pragma once

#include "core/list.h"
#include "core/memory/types.h"
#include "resource/types.h"
#include "world/types.h"

namespace crown
{
/// Game level.
///
/// @ingroup World
struct Level
{
	u32 _marker;
	const LevelResource *_resource;
	UnitId *_unit_lookup;
	ListNode _node;

	///
	Level(const LevelResource *level_resource, UnitId *unit_lookup);

	///
	~Level();

	///
	UnitId unit_by_name(StringId32 name);
};

namespace level
{
	///
	Level *create(Allocator &a, const LevelResource *level_resource);

	///
	void destroy(Allocator &a, Level *level);

} // namespace level

} // namespace crown
