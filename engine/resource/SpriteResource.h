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

#include "Types.h"
#include "Allocator.h"
#include "File.h"
#include "OS.h"
#include "StringUtils.h"
#include "List.h"
#include "Bundle.h"
#include "Device.h"
#include "RendererTypes.h"
#include "Renderer.h"
#include "ResourceManager.h"
#include "TextureResource.h"
#include "SpriteAnimator.h"

#define MAX_SPRITE_ANIM_FRAMES 60
#define MAX_VERTEX_SIZE_PER_FRAME 16

namespace crown
{

const uint32_t SPRITE_VERSION = 1;

//-----------------------------------------------------------------------------
struct SpriteResourceData
{
	char 		m_name[128];
	char 		m_texture[128];
	uint32_t	m_length;
	uint32_t	m_frame_rate;
	float		m_vertices[MAX_SPRITE_ANIM_FRAMES * MAX_VERTEX_SIZE_PER_FRAME];
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

		sr->m_vb = device()->renderer()->create_vertex_buffer(4, VertexFormat::P2_T2, sr->vertices());
		sr->m_ib = device()->renderer()->create_index_buffer(6, t_indices);

		TextureResource* res = (TextureResource*)device()->resource_manager()->lookup(TEXTURE_EXTENSION, sr->texture());
		sr->m_texture = res->m_texture;
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

		device()->renderer()->destroy_vertex_buffer(sprite->m_vb);
		device()->renderer()->destroy_index_buffer(sprite->m_ib);
	}

private:
	//-----------------------------------------------------------------------------
	const char* name()
	{
		SpriteResourceData* t_data = (SpriteResourceData*)m_data;
		return t_data->m_name;
	}

	//-----------------------------------------------------------------------------
	const char* texture()
	{
		SpriteResourceData* t_data = (SpriteResourceData*)m_data;
		return t_data->m_texture;		
	}

	//-----------------------------------------------------------------------------
	uint32_t length()
	{
		SpriteResourceData* t_data = (SpriteResourceData*)m_data;
		return t_data->m_length;
	}

	//-----------------------------------------------------------------------------
	uint32_t frame_rate()
	{
		SpriteResourceData* t_data = (SpriteResourceData*)m_data;
		return t_data->m_frame_rate;
	}

	//-----------------------------------------------------------------------------
	float* vertices()
	{
		SpriteResourceData* t_data = (SpriteResourceData*)m_data;
		return t_data->m_vertices;
	}

public:

	uint8_t*					m_data;
	size_t						m_data_size;

	TextureId 					m_texture;
	VertexBufferId 				m_vb;
	IndexBufferId 				m_ib;
};

} // namespace crown