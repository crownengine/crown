/*
 * Copyright (c) 2012-2017 Daniele Bartolini and individual contributors.
 * License: https://github.com/taylor001/crown/blob/master/LICENSE
 */

#pragma once

#include "compiler_types.h"
#include "filesystem_types.h"
#include "math_types.h"
#include "memory_types.h"
#include "resource_types.h"
#include "string_id.h"

namespace crown
{
struct LevelResource
{
	u32 version;
	u32 units_offset;
	u32 num_sounds;
	u32 sounds_offset;
};

struct LevelSound
{
	StringId64 name;
	Vector3 position;
	f32 volume;
	f32 range;
	bool loop;
	char _pad[3];
};

namespace level_resource_internal
{
	void compile(const char* path, CompileOptions& opts);

} // namespace level_resource_internal

namespace level_resource
{
	/// Returns the unit resource in the level.
	const UnitResource* unit_resource(const LevelResource* lr);

	/// Returns the number of sounds in the level resource.
	u32 num_sounds(const LevelResource* lr);

	/// Returns the sound @a i.
	const LevelSound* get_sound(const LevelResource* lr, u32 i);

} // namespace level_resource

} // namespace crown
