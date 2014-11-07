/*
Copyright (c) 2013 Daniele Bartolini, Michele Rossi
Copyright (c) 2012 Daniele Bartolini, Simone Boscaratto

Permission is hereby granted, free of charge, to any person
obtaining a copy of this software and associated documentation
files (the "Software"), to deal in the Software without
restriction, including without limitation the rights to use,
copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the
Software is furnished to do so, subject to the following
conditions:

The above copyright notice and this permission notice shall be
included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
OTHER DEALINGS IN THE SOFTWARE.
*/

#pragma once

#include "types.h"
#include "memory_types.h"
#include "resource_types.h"
#include "filesystem_types.h"
#include "compiler_types.h"

namespace crown
{

struct SoundType
{
	enum Enum
	{
		WAV = 0,
		OGG = 1
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
