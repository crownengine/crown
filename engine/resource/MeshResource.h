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

class MeshResource
{
public:

	//-----------------------------------------------------------------------------
	static void* load(Allocator& allocator, Bundle& bundle, ResourceId id)
	{
		File* file = bundle.open(id);

		const size_t file_size = file->size() - 12;
		MeshResource* res = (MeshResource*) allocator.allocate(sizeof(MeshResource));
		res->m_data = (uint8_t*) allocator.allocate(file_size);
		file->read(res->m_data, file_size);

		bundle.close(file);

		return res;
	}

	//-----------------------------------------------------------------------------
	static void online(void* resource)
	{
		MeshResource* m = (MeshResource*) resource;

		m->m_vbuffer = device()->renderer()->create_vertex_buffer(m->num_vertices(), m->vertex_format(), m->vertices());
		m->m_ibuffer = device()->renderer()->create_index_buffer(m->num_indices(), m->indices());
	}

	//-----------------------------------------------------------------------------
	static void unload(Allocator& a, void* res)
	{
		MeshResource* resource = (MeshResource*)res;
		a.deallocate(resource->m_data);
		a.deallocate(resource);
	}

	//-----------------------------------------------------------------------------
	static void offline(void* resource)
	{
		MeshResource* m = (MeshResource*) resource;

		device()->renderer()->destroy_index_buffer(m->m_ibuffer);
		device()->renderer()->destroy_vertex_buffer(m->m_vbuffer);
	}

	//-----------------------------------------------------------------------------
	uint32_t num_vertices()
	{
		MeshData* data = (MeshData*) (m_data + sizeof(MeshHeader));
		return data->vertices.num_vertices;
	}

	//-----------------------------------------------------------------------------
	VertexFormat::Enum vertex_format()
	{
		MeshData* data = (MeshData*) (m_data + sizeof(MeshHeader));
		return data->vertices.format;
	}

	//-----------------------------------------------------------------------------
	float* vertices()
	{
		MeshData* data = (MeshData*) (m_data + sizeof(MeshHeader));
		return (float*) (m_data + data->vertices.offset);
	}

	//-----------------------------------------------------------------------------
	uint32_t num_indices()
	{
		MeshData* data = (MeshData*) (m_data + sizeof(MeshHeader));
		return data->indices.num_indices;
	}

	//-----------------------------------------------------------------------------
	uint16_t* indices()
	{
		MeshData* data = (MeshData*) (m_data + sizeof(MeshHeader));
		return (uint16_t*) (m_data + data->indices.offset);
	}

public:

	uint8_t* m_data;

	VertexBufferId m_vbuffer;
	IndexBufferId m_ibuffer;
};

} // namespace crown
