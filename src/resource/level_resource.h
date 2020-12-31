/*
 * Copyright (c) 2012-2021 Daniele Bartolini et al.
 * License: https://github.com/dbartolini/crown/blob/master/LICENSE
 */

#pragma once

#include "core/filesystem/types.h"
#include "core/math/types.h"
#include "core/memory/types.h"
#include "core/strings/string_id.h"
#include "resource/types.h"
#include "resource/types.h"

namespace crown
{
struct LevelResource
{
	u32 version;
	u32 num_units;
	u32 unit_names_offset;
	u32 units_offset;
	u32 num_sounds;
	u32 sounds_offset;
	// StringId32[num_units]  <-- unit_names_offset
	// UnitResource           <-- units_offset
	// LevelSound[num_sounds] <-- sounds_offset
};

struct LevelSound
{
	StringId64 name;
	Vector3 position;
	f32 volume;
	f32 range;
	u32 loop;
};

namespace level_resource_internal
{
	s32 compile(CompileOptions& opts);

} // namespace level_resource_internal

namespace level_resource
{
	/// Returns the unit names in the level.
	const StringId32* unit_names(const LevelResource* lr);

	/// Returns the unit resource in the level.
	const UnitResource* unit_resource(const LevelResource* lr);

	/// Returns the number of sounds in the level resource.
	u32 num_sounds(const LevelResource* lr);

	/// Returns the sound @a i.
	const LevelSound* get_sound(const LevelResource* lr, u32 i);

} // namespace level_resource

} // namespace crown
