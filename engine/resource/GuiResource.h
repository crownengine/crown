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

#include "Assert.h"
#include "Types.h"
#include "Allocator.h"
#include "File.h"
#include "OS.h"
#include "StringUtils.h"
#include "List.h"
#include "Bundle.h"
#include "Device.h"
#include "Renderer.h"
#include "Vector2.h"
#include "Vector3.h"

namespace crown
{

const uint32_t GUI_RESOURCE_VERSION = 1;

//-----------------------------------------------------------------------------
struct GuiHeader
{
	uint32_t position[2];
	uint32_t size[2];

	VertexBufferId rect_vb;
	IndexBufferId rect_ib;
	VertexBufferId triangle_vb;
	IndexBufferId triangle_ib;

	uint32_t num_rects;
	uint32_t num_triangles;

	uint32_t rect_vertices_off;
	uint32_t rect_indices_off;
	uint32_t triangle_vertices_off;
	uint32_t triangle_indices_off;
};

//-----------------------------------------------------------------------------
struct GuiResource
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
	static void online(void* resource)
	{
		GuiHeader* h = (GuiHeader*) resource;

		const float* rect_vertices = (float*) (((char*) resource) + h->rect_vertices_off);
		const uint16_t* rect_indices = (uint16_t*) (((char*) resource) + h->rect_indices_off);

		h->rect_vb = device()->renderer()->create_vertex_buffer(h->num_rects * 4, VertexFormat::P2_C4, rect_vertices);
		h->rect_ib = device()->renderer()->create_index_buffer(h->num_rects * 8, rect_indices);

		const float* triangle_vertices = (float*) (((char*) resource) + h->triangle_vertices_off);
		const uint16_t* triangle_indices = (uint16_t*) (((char*) resource) + h->triangle_indices_off);

		h->triangle_vb = device()->renderer()->create_vertex_buffer(h->num_triangles * 3, VertexFormat::P2_C4, triangle_vertices);
		h->triangle_ib = device()->renderer()->create_index_buffer(h->num_triangles * 6, triangle_indices);
	}

	//-----------------------------------------------------------------------------
	static void unload(Allocator& allocator, void* resource)
	{
		CE_ASSERT_NOT_NULL(resource);
		allocator.deallocate(resource);
	}

	//-----------------------------------------------------------------------------
	static void offline(void* resource)
	{
		CE_ASSERT_NOT_NULL(resource);

		GuiResource* gr = (GuiResource*) resource;
		GuiHeader* h = (GuiHeader*) gr;

		device()->renderer()->destroy_vertex_buffer(h->triangle_vb);
		device()->renderer()->destroy_index_buffer(h->triangle_ib);
		device()->renderer()->destroy_vertex_buffer(h->rect_vb);
		device()->renderer()->destroy_index_buffer(h->rect_ib);
	}

	//-----------------------------------------------------------------------------
	Vector3 gui_position() const
	{
		Vector3 pos;
		
		pos.x = ((GuiHeader*)this)->position[0];
		pos.y = ((GuiHeader*)this)->position[1];
		pos.z = 0.0f;

		return pos;
	}

	//-----------------------------------------------------------------------------
	Vector2 gui_size() const
	{
		Vector2 size;

		size.x = ((GuiHeader*)this)->size[0];
		size.y = ((GuiHeader*)this)->size[1];

		return size;
	}

	//-----------------------------------------------------------------------------
	VertexBufferId rect_vertex_buffer() const
	{
		return ((GuiHeader*)this)->rect_vb;
	}

	//-----------------------------------------------------------------------------
	IndexBufferId rect_index_buffer() const
	{
		return ((GuiHeader*)this)->rect_ib;
	}

	//-----------------------------------------------------------------------------
	VertexBufferId triangle_vertex_buffer() const
	{
		return ((GuiHeader*)this)->triangle_vb;
	}

	//-----------------------------------------------------------------------------
	IndexBufferId triangle_index_buffer() const
	{
		return ((GuiHeader*)this)->triangle_ib;
	}


private:
	
	GuiResource();
};

} // namespace crown