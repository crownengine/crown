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
	uint32_t num_rects;
	uint32_t num_triangles;
	uint32_t num_images;
	uint32_t rects_offset;
	uint32_t triangles_offset;
	uint32_t images_offset;
};

//-----------------------------------------------------------------------------
struct GuiRectData
{
	float		position[3];
	float		size[2];
	float		color[4];
};

//-----------------------------------------------------------------------------
struct GuiTriangleData
{
	float		points[6];
	float		color[4];
};

//-----------------------------------------------------------------------------
struct GuiImageData
{
	ResourceId 	material;
	float		position[3];
	float		size[2];
	float		color[4];
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
	uint32_t num_rects() const
	{
		return ((GuiHeader*)this)->num_rects;
	}

	//-----------------------------------------------------------------------------
	GuiRectData get_rect(uint32_t index) const
	{
		CE_ASSERT(index < num_rects(), "Index out of bounds");

		GuiHeader* h = (GuiHeader*) this;
		GuiRectData* begin = (GuiRectData*) (((char*) this) + h->rects_offset);
		return begin[index];	
	}

	//-----------------------------------------------------------------------------
	uint32_t num_triangles() const
	{
		return ((GuiHeader*)this)->num_triangles;
	}

	//-----------------------------------------------------------------------------
	GuiTriangleData get_triangle(uint32_t index) const
	{
		CE_ASSERT(index < num_triangles(), "Index out of bounds");

		GuiHeader* h = (GuiHeader*) this;
		GuiTriangleData* begin = (GuiTriangleData*) (((char*) this) + h->triangles_offset);
		return begin[index];
	}

	//-----------------------------------------------------------------------------
	uint32_t num_images() const
	{
		return ((GuiHeader*)this)->num_images;
	}

	//-----------------------------------------------------------------------------
	GuiImageData get_image(uint32_t index) const
	{
		CE_ASSERT(index < num_images(), "Index out of bounds");

		GuiHeader* h = (GuiHeader*) this;
		GuiImageData* begin = (GuiImageData*) (((char*) this) + h->images_offset);
		return begin[index];
	}

private:
	
	GuiResource();
};

} // namespace crown