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
#include "resource.h"
#include "bundle.h"
#include "allocator.h"
#include "file.h"
#include "device.h"

namespace crown
{

//-----------------------------------------------------------------------------
struct SoundType
{
	enum Enum
	{
		WAV = 0,
		OGG = 1
	};
};

const uint32_t SOUND_VERSION = 1;

//-----------------------------------------------------------------------------
struct SoundHeader
{
	uint32_t		version;	// Sound file version
	uint32_t		size;
	uint32_t		sample_rate;
	uint32_t		avg_bytes_ps;
	uint32_t		channels;
	uint16_t		block_size;
	uint16_t 		bits_ps;
	uint8_t			sound_type;
};

struct SoundResource
{
	uint32_t size() const
	{
		return ((SoundHeader*) this)->size;
	}

	uint32_t sample_rate() const
	{
		return ((SoundHeader*) this)->sample_rate;
	}

	uint32_t avg_bytes_ps() const
	{
		return ((SoundHeader*) this)->avg_bytes_ps;
	}

	uint32_t channels() const
	{
		return ((SoundHeader*) this)->channels;
	}

	uint16_t block_size() const
	{
		return ((SoundHeader*) this)->block_size;
	}

	uint16_t bits_ps() const
	{
		return ((SoundHeader*) this)->bits_ps;
	}

	uint8_t sound_type() const
	{
		return ((SoundHeader*) this)->sound_type;
	}

	const char* data() const
	{
		return ((char*) this) + sizeof(SoundHeader);
	}

private:

	// Disable construction
	SoundResource();
};

namespace sound_resource
{
	void compile(Filesystem& fs, const char* resource_path, File* out_file);
	inline void compile(const char* path, CompileOptions& opts)
	{
		compile(opts._fs, path, &opts._bw.m_file);
	}
	void* load(Allocator& allocator, Bundle& bundle, ResourceId id);
	void online(StringId64 /*id*/, ResourceManager& /*rm*/);
	void offline(StringId64 /*id*/, ResourceManager& /*rm*/);
	void unload(Allocator& allocator, void* resource);
} // namespace sound_resource
} // namespace crown
