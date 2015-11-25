/*
 * Copyright (c) 2012-2015 Daniele Bartolini and individual contributors.
 * License: https://github.com/taylor001/crown/blob/master/LICENSE
 */

#pragma once

#include "types.h"
#include "memory_types.h"
#include "resource_types.h"
#include "filesystem_types.h"
#include "compiler_types.h"
#include "string_id.h"

namespace crown
{

struct SoundType
{
	enum Enum
	{
		WAV,
		OGG
	};
};

struct SoundResource
{
	uint32_t version;
	uint32_t size;
	uint32_t sample_rate;
	uint32_t avg_bytes_ps;
	uint32_t channels;
	uint16_t block_size;
	uint16_t bits_ps;
	uint8_t sound_type;
	char _pad[3];
};

namespace sound_resource
{
	void compile(const char* path, CompileOptions& opts);
	void* load(File& file, Allocator& a);
	void online(StringId64 /*id*/, ResourceManager& /*rm*/);
	void offline(StringId64 /*id*/, ResourceManager& /*rm*/);
	void unload(Allocator& allocator, void* resource);

	uint32_t size(const SoundResource* sr);
	uint32_t sample_rate(const SoundResource* sr);
	uint32_t avg_bytes_ps(const SoundResource* sr);
	uint32_t channels(const SoundResource* sr);
	uint16_t block_size(const SoundResource* sr);
	uint16_t bits_ps(const SoundResource* sr);
	uint8_t sound_type(const SoundResource* sr);
	const char* data(const SoundResource* sr);
} // namespace sound_resource
} // namespace crown
