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

#include <cstring>
#include <inttypes.h>

#include "Types.h"
#include "Allocator.h"
#include "File.h"
#include "OS.h"
#include "StringUtils.h"
#include "List.h"
#include "Bundle.h"
#include "Device.h"
#include "ResourceManager.h"
#include "RendererTypes.h"
#include "Renderer.h"
#include "TextureResource.h"

#define MAX_SPRITE_ANIM_FRAMES 60
#define SPRITE_FRAME_SIZE 4 * 4 * 4

namespace crown
{

const uint32_t SPRITE_VERSION = 1;

//-----------------------------------------------------------------------------
struct SpriteHeader
{
	char 		name[128];
	ResourceId 	texture;
	uint32_t 	num_frames;
	uint32_t 	frame_rate;
	uint32_t 	playback_mode;
};

//-----------------------------------------------------------------------------
class SpriteResource
{
public:

	//-----------------------------------------------------------------------------
	static void* load(Allocator& allocator, Bundle& bundle, ResourceId id)
	{
		File* file = bundle.open(id);

		const size_t file_size = file->size() - 12;

		SpriteResource* res = (SpriteResource*) allocator.allocate(sizeof(SpriteResource));
		res->m_data = (uint8_t*) allocator.allocate(file_size);
		res->m_data_size = file_size;
		file->read(res->m_data, res->m_data_size);

		bundle.close(file);

		return res;
	}

	//-----------------------------------------------------------------------------
	static void online(void* resource)
	{
		SpriteResource* sr = (SpriteResource*)resource;

		static uint16_t t_indices[] = {0, 1, 2, 0, 2, 3};

		sr->m_vb = device()->renderer()->create_vertex_buffer(4, VertexFormat::P2_T2, sr->frame(0));
		sr->m_ib = device()->renderer()->create_index_buffer(6, t_indices);

		TextureResource* res = (TextureResource*)device()->resource_manager()->data(sr->texture());
		sr->m_texture = res->m_texture;

		Log::i("resource: " "%.16"PRIx64"", sr->texture().id);
	}

	//-----------------------------------------------------------------------------
	static void unload(Allocator& allocator, void* resource)
	{
		SpriteResource* res = (SpriteResource*)resource;
		allocator.deallocate(res->m_data);
		allocator.deallocate(res);
	}

	//-----------------------------------------------------------------------------
	static void offline(void* resource)
	{
		SpriteResource* sprite = (SpriteResource*) resource;

		Renderer* r = device()->renderer();

		r->destroy_vertex_buffer(sprite->m_vb);
		r->destroy_index_buffer(sprite->m_ib);
	}

	//-----------------------------------------------------------------------------
	const char* name()
	{
		SpriteHeader* header = (SpriteHeader*)m_data;
		return header->name;
	}

	//-----------------------------------------------------------------------------
	ResourceId texture()
	{
		SpriteHeader* header = (SpriteHeader*)m_data;
		return header->texture;
	}

	//-----------------------------------------------------------------------------
	uint32_t num_frames()
	{
		SpriteHeader* header = (SpriteHeader*)m_data;
		return header->num_frames;
	}

	//-----------------------------------------------------------------------------
	uint32_t frame_rate()
	{
		SpriteHeader* header = (SpriteHeader*)m_data;
		return header->frame_rate;
	}

	//-----------------------------------------------------------------------------
	uint32_t playback_mode()
	{
		SpriteHeader* header = (SpriteHeader*)m_data;
		return header->playback_mode;
	}

	//-----------------------------------------------------------------------------
	float* animation()
	{
		return (float*)(m_data + sizeof(SpriteHeader));
	}

	float* frame(uint32_t index)
	{
		return (float*)(m_data + sizeof(SpriteHeader) + SPRITE_FRAME_SIZE * index);
	}

public:

	uint8_t*					m_data;
	size_t						m_data_size;

	TextureId 					m_texture;
	VertexBufferId 				m_vb;
	IndexBufferId 				m_ib;
};

} // namespace crown