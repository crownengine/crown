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
#include "PixelFormat.h"
#include "Allocator.h"
#include "Bundle.h"
#include "File.h"
#include "Device.h"
#include "Renderer.h"

namespace crown
{

// Bump the version whenever a change in the format is made.
const uint32_t MESH_VERSION = 1;

struct MeshHeader
{
	VertexBufferId		vbuffer;
	IndexBufferId		ibuffer;
	uint32_t			version;
	uint32_t			num_meshes;
	uint32_t			num_joints;
	uint32_t			padding[16];
};

struct VertexData
{
	uint32_t	    	num_vertices;
	VertexFormat::Enum	format;
	uint32_t			offset;
};

struct IndexData
{
	uint32_t			num_indices;
	uint32_t			offset;
};

struct MeshData
{
	VertexData			vertices;
	IndexData			indices;
};

struct MeshResource
{
public:

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
		MeshResource* m = (MeshResource*) resource;
		MeshHeader* h = (MeshHeader*) m;

		h->vbuffer = device()->renderer()->create_vertex_buffer(m->num_vertices(), m->vertex_format(), m->vertices());
		h->ibuffer = device()->renderer()->create_index_buffer(m->num_indices(), m->indices());
	}

	//-----------------------------------------------------------------------------
	static void unload(Allocator& a, void* res)
	{
		CE_ASSERT_NOT_NULL(res);
		a.deallocate(res);
	}

	//-----------------------------------------------------------------------------
	static void offline(void* resource)
	{
		MeshResource* m = (MeshResource*) resource;
		MeshHeader* h = (MeshHeader*) m;

		device()->renderer()->destroy_index_buffer(h->ibuffer);
		device()->renderer()->destroy_vertex_buffer(h->vbuffer);
	}

	//-----------------------------------------------------------------------------
	uint32_t num_vertices() const
	{
		MeshData* data = (MeshData*) ((char*) this) + sizeof(MeshHeader);
		return data->vertices.num_vertices;
	}

	//-----------------------------------------------------------------------------
	VertexFormat::Enum vertex_format() const
	{
		MeshData* data = (MeshData*) ((char*) this) + sizeof(MeshHeader);
		return data->vertices.format;
	}

	//-----------------------------------------------------------------------------
	float* vertices() const
	{
		MeshData* data = (MeshData*) ((char*) this) + sizeof(MeshHeader);
		return (float*) (((char*)this) + data->vertices.offset);
	}

	//-----------------------------------------------------------------------------
	uint32_t num_indices() const
	{
		MeshData* data = (MeshData*) ((char*) this) + sizeof(MeshHeader);
		return data->indices.num_indices;
	}

	//-----------------------------------------------------------------------------
	uint16_t* indices() const
	{
		MeshData* data = (MeshData*) ((char*) this) + sizeof(MeshHeader);
		return (uint16_t*) (((char*)this) + data->indices.offset);
	}

	//-----------------------------------------------------------------------------
	VertexBufferId vertex_buffer() const
	{
		MeshHeader* h = (MeshHeader*) this;
		return h->vbuffer;
	}

	//-----------------------------------------------------------------------------
	IndexBufferId index_buffer() const
	{
		MeshHeader* h = (MeshHeader*) this;
		return h->ibuffer;
	}

private:

	// Disable construction
	MeshResource();
};

} // namespace crown
