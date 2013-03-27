/*
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
#include "List.h"
#include "Box.h"
#include "MeshChunk.h"
#include "Resource.h"

namespace crown
{

class VertexBuffer;
class IndexBuffer;

/**
 * This class represents a mesh in order to define the shape of a generic 'object'.
 * Mesh can have an arbitrary number of sub-meshes called MeshChunk which contribute
 * to the final mesh structure.
 */ 
class MeshResource
{

	typedef List<MeshChunk*> MeshChunkList;

public:


							MeshResource();	//! Constructor
							~MeshResource();	//! Destructor


	uint32_t				GetMeshChunkCount() const;	//! Returns the MeshChunks contained in this mesh
	MeshChunk*				GetMeshChunk(uint32_t index) const;
	void					AddMeshChunk(MeshChunk* chunk);	//! Adds a MeshChunk to the mesh
	void					RemoveMeshChunk(MeshChunk* chunk);	//! Remove a MeshChunk from the mesh

	const Box&				GetBoundingBox() const;	//! Returns the mesh's bounding box
	void					UpdateBoundingBox();	//! Recalculate the mesh's bounding box

	inline VertexBuffer*	GetVertexBuffer() const { return mVertexBuffer; }
	inline IndexBuffer*		GetIndexBuffer() const { return mIndexBuffer; }

	inline uint32_t			GetGlobalVertexCount() const { return mGlobalVertexCount; }

	void					RecompileMesh();

private:

	MeshChunkList			mMeshChunkList;

	Box						mBoundingBox;

	VertexBuffer*			mVertexBuffer;
	IndexBuffer*			mIndexBuffer;
	uint32_t				mGlobalVertexCount;
};

} // namespace crown

