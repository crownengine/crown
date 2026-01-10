/*
 * Copyright (c) 2012-2026 Daniele Bartolini et al.
 * SPDX-License-Identifier: MIT
 */

#include "resource/level_resource.h"

namespace crown
{
namespace level_resource
{
	const StringId32 *unit_names(const LevelResource *lr)
	{
		return (StringId32 *)((char *)lr + lr->unit_names_offset);
	}

	const UnitResource *unit_resource(const LevelResource *lr)
	{
		return (UnitResource *)((char *)lr + lr->units_offset);
	}

	u32 num_sounds(const LevelResource *lr)
	{
		return lr->num_sounds;
	}

	const LevelSound *get_sound(const LevelResource *lr, u32 i)
	{
		CE_ASSERT(i < num_sounds(lr), "Index out of bounds");
		const LevelSound *begin = (LevelSound *)((char *)lr + lr->sounds_offset);
		return &begin[i];
	}

} // namespace level_resource

} // namespace crown
