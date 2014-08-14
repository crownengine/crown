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
#include "allocator.h"
#include "bundle.h"
#include "file.h"

namespace crown
{

// Bump the version whenever a change in the format is made.
const uint32_t MESH_VERSION = 1;

struct MeshHeader
{
	uint32_t			vbuffer;
	uint32_t			ibuffer;
	uint32_t			version;
	uint32_t			num_meshes;
	uint32_t			num_joints;
	uint32_t			padding[16];
};

struct VertexData
{
	uint32_t	    	num_vertices;
	// VertexFormat::Enum	format;
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
	static void online(StringId64 /*id*/, ResourceManager& /*rm*/)
	{
	}

	static void offline(StringId64 /*id*/, ResourceManager& /*rm*/)
	{
	}

	//-----------------------------------------------------------------------------
	static void unload(Allocator& a, void* res)
	{
		CE_ASSERT_NOT_NULL(res);
		a.deallocate(res);
	}

	//-----------------------------------------------------------------------------
	uint32_t num_vertices() const
	{
		MeshData* data = (MeshData*) ((char*) this) + sizeof(MeshHeader);
		return data->vertices.num_vertices;
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

private:

	// Disable construction
	MeshResource();
};

} // namespace crown
