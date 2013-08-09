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

#include "MeshResource.h"
#include "Bundle.h"
#include "Log.h"
#include "DiskFile.h"
#include "Assert.h"
#include "Allocator.h"
#include "Device.h"
#include "Renderer.h"

namespace crown
{

//-----------------------------------------------------------------------------
void* MeshResource::load(Allocator& allocator, Bundle& bundle, ResourceId id)
{
	DiskFile* file = bundle.open(id);

	CE_ASSERT(file != NULL, "Resource does not exist: %.8X%.8X", id.name, id.type);

	MeshResource* resource = (MeshResource*)allocator.allocate(sizeof(MeshResource));
	file->read(&resource->m_header, sizeof(MeshHeader));

	// Read vertices
	file->read(&resource->m_vertex_count, sizeof(uint32_t));
	resource->m_vertices = (float*) allocator.allocate(sizeof(float) * resource->m_vertex_count);
	file->read(resource->m_vertices, sizeof(float) * resource->m_vertex_count);

	// Read triangles
	file->read(&resource->m_index_count, sizeof(uint32_t));
	resource->m_indices = (uint16_t*) allocator.allocate(sizeof(uint16_t) * resource->m_index_count);
	file->read(resource->m_indices, sizeof(uint16_t) * resource->m_index_count);

	bundle.close(file);

	return resource;
}

//-----------------------------------------------------------------------------
void MeshResource::online(void* )
{
}

//-----------------------------------------------------------------------------
void MeshResource::unload(Allocator& , void* )
{
}

//-----------------------------------------------------------------------------
void MeshResource::offline(void* /*resource*/)
{

}

} // namespace crown
