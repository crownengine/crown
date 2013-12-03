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

#include "Types.h"
#include "Resource.h"
#include "Bundle.h"
#include "Allocator.h"
#include "File.h"

namespace crown
{

// All offsets are absolute
struct UnitHeader
{
	uint32_t num_renderables;
	uint32_t renderables_offset;
	uint32_t num_cameras;
	uint32_t cameras_offset;
	uint32_t num_actors;
	uint32_t actors_offset;
};

struct UnitRenderable
{
	enum { MESH, SPRITE } type;
	ResourceId resource;
	uint32_t name;
	bool visible;
};

struct UnitCamera
{
	uint32_t name;
};

struct UnitActor
{
	uint32_t name;
	enum {STATIC, DYNAMIC, UNK_TYPE} type;
	enum {SPHERE, BOX, PLANE, UNK_SHAPE} shape;
	bool active;
};

class UnitResource
{
public:

	//-----------------------------------------------------------------------------
	static void* load(Allocator& allocator, Bundle& bundle, ResourceId id)
	{
		File* file = bundle.open(id);

		const size_t file_size = file->size() - 12;
		UnitResource* res = (UnitResource*) allocator.allocate(sizeof(UnitResource));
		res->m_data = (char*) allocator.allocate(file_size);
		file->read(res->m_data, file_size);

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

		allocator.deallocate(((UnitResource*)resource)->m_data);
		allocator.deallocate(resource);
	}

	//-----------------------------------------------------------------------------
	static void offline(void* /*resource*/)
	{
	}

public:

	//-----------------------------------------------------------------------------
	uint32_t num_renderables() const
	{
		CE_ASSERT_NOT_NULL(m_data);

		return ((UnitHeader*)m_data)->num_renderables;
	}

	//-----------------------------------------------------------------------------
	const UnitRenderable& get_renderable(uint32_t i) const
	{
		CE_ASSERT(i < num_renderables(), "Index out of bounds");

		UnitRenderable* begin = (UnitRenderable*) (m_data + ((UnitHeader*)m_data)->renderables_offset);
		return begin[i];
	}

	//-----------------------------------------------------------------------------
	uint32_t num_cameras() const
	{
		CE_ASSERT_NOT_NULL(m_data);

		return ((UnitHeader*)m_data)->num_cameras;
	}

	//-----------------------------------------------------------------------------
	const UnitCamera& get_camera(uint32_t i) const
	{
		CE_ASSERT(i < num_cameras(), "Index out of bounds");

		UnitCamera* begin = (UnitCamera*) (m_data + ((UnitHeader*)m_data)->cameras_offset);
		return begin[i];
	}

	//-----------------------------------------------------------------------------
	uint32_t num_actors() const
	{
		CE_ASSERT_NOT_NULL(m_data);

		return ((UnitHeader*)m_data)->num_actors;
	}

	//-----------------------------------------------------------------------------
	const UnitActor& get_actor(uint32_t i) const
	{
		CE_ASSERT(i < num_actors(), "Index out of bounds");

		UnitActor* begin = (UnitActor*)(m_data + ((UnitHeader*)m_data)->actors_offset);
		return begin[i];
	}

private:

	char* m_data;
};

} // namespace crown
