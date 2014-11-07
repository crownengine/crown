/*
 * Copyright (c) 2012-2014 Daniele Bartolini and individual contributors.
 * License: https://github.com/taylor001/crown/blob/master/LICENSE
 */

#pragma once

#include "memory_types.h"
#include "filesystem_types.h"
#include "math_types.h"
#include "resource_types.h"
#include "compiler_types.h"

namespace crown
{

struct LevelResource
{
	uint32_t version;
	uint32_t num_units;
	uint32_t units_offset;
	uint32_t num_sounds;
	uint32_t sounds_offset;
};

struct LevelUnit
{
	StringId64 name;
	Vector3 position;
	Quaternion rotation;
	uint32_t _pad;
};

struct LevelSound
{
	StringId64 name;
	Vector3 position;
	float volume;
	float range;
	bool loop;
	char _pad[3];
};

namespace level_resource
{
	void compile(const char* path, CompileOptions& opts);
	void* load(File& file, Allocator& a);
	void online(StringId64 /*id*/, ResourceManager& /*rm*/);
	void offline(StringId64 /*id*/, ResourceManager& /*rm*/);
	void unload(Allocator& allocator, void* resource);

	uint32_t num_units(const LevelResource* lr);
	const LevelUnit* get_unit(const LevelResource* lr, uint32_t i);
	uint32_t num_sounds(const LevelResource* lr);
	const LevelSound* get_sound(const LevelResource* lr, uint32_t i);
} // namespace level_resource
} // namespace crown
