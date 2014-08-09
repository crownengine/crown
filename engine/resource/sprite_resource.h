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
#include "allocator.h"
#include "file.h"
#include "os.h"
#include "string_utils.h"
#include "container_types.h"
#include "bundle.h"
#include "device.h"
#include "resource_manager.h"
#include "texture_resource.h"
#include "blob.h"
#include <cstring>
#include <inttypes.h>
#include <bgfx.h>

namespace crown
{

const uint32_t SPRITE_VERSION = 1;

struct SpriteHeader
{
	uint32_t		dummy[2];
	uint32_t		num_animations;
	uint32_t		animations_offset;
	uint32_t		num_vertices;
	uint32_t		vertices_offset;
	uint32_t		num_indices;
	uint32_t		indices_offset;
};

struct SpriteAnimation
{
	StringId32 name;
	float time;
	uint32_t num_frames;
	uint32_t start_frame;
};

//-----------------------------------------------------------------------------
struct SpriteResource
{
	//-----------------------------------------------------------------------------
	static void* load(Allocator& allocator, Bundle& bundle, ResourceId id)
	{
		File* file = bundle.open(id);
		const size_t file_size = file->size();

		void* res = allocator.allocate(file_size);
		file->read(res, file_size);

		bundle.close(file);

		return res;
	}

	//-----------------------------------------------------------------------------
	static void online(void* /*resource*/)
	{
	}

	//-----------------------------------------------------------------------------
	static void unload(Allocator& allocator, void* resource)
	{
		CE_ASSERT_NOT_NULL(resource);
		allocator.deallocate(resource);
	}

	//-----------------------------------------------------------------------------
	static void offline(void* /*resource*/)
	{
	}

	//-----------------------------------------------------------------------------
	const SpriteAnimation* get_animation(const char* name) const
	{
		SpriteHeader* h = (SpriteHeader*) this;
		SpriteAnimation* begin = (SpriteAnimation*) (((char*) this) + h->animations_offset);

		const StringId32 name_hash = string::murmur2_32(name, string::strlen(name));
		const uint32_t num = h->num_animations;
		for (uint32_t i = 0; i < num; i++)
		{
			if (begin[i].name == name_hash)
				return &begin[i];
		}

		return NULL;
	}

	uint32_t get_animation_frame(const SpriteAnimation* a, uint32_t frame) const
	{
		SpriteHeader* h = (SpriteHeader*) this;
		uint32_t* begin = (uint32_t*) (((char*) this) + h->animations_offset + sizeof(SpriteAnimation) * h->num_animations);
		return begin[a->start_frame + frame];
	}

	Blob get_vertices() const
	{
		SpriteHeader* h = (SpriteHeader*) this;
		Blob b;
		b.m_size = ((SpriteHeader*) this)->num_vertices * 16;
		b.m_data = (uintptr_t) (((char*) this) + h->vertices_offset);
		return b;
	}

	Blob get_indices() const
	{
		SpriteHeader* h = (SpriteHeader*) this;
		Blob b;
		b.m_size = ((SpriteHeader*) this)->num_indices * 2;
		b.m_data = (uintptr_t) (((char*) this) + h->indices_offset);
		return b;
	}

private:

	// Disable construction
	SpriteResource();
};

} // namespace crown
