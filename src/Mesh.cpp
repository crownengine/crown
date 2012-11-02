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

#include "Data.h"
#include "Mesh.h"
#include "Device.h"
#include "Types.h"
#include "VertexBuffer.h"
#include "IndexBuffer.h"
#include "Renderer.h"

namespace Crown
{

Mesh::Mesh() :
	mVertexBuffer(NULL),
	mIndexBuffer(NULL),
	mGlobalVertexCount(0)
{
}

Mesh::~Mesh()
{
	for (int i = 0; i < mMeshChunkList.GetSize(); i++)
	{
		delete mMeshChunkList[i];
	}
}

uint Mesh::GetMeshChunkCount() const
{
	return mMeshChunkList.GetSize();
}

MeshChunk* Mesh::GetMeshChunk(uint index) const
{
	return mMeshChunkList[index];
}

void Mesh::AddMeshChunk(MeshChunk* chunk)
{
	if (!chunk)
	{
		return;
	}

	mBoundingBox.AddBox(chunk->GetBoundingBox());
	mMeshChunkList.Append(chunk);
}

void Mesh::RemoveMeshChunk(MeshChunk* chunk)
{
	for (int i = 0; i < mMeshChunkList.GetSize(); i++)
	{
		if (mMeshChunkList[i] == chunk)
		{
			mMeshChunkList.Remove(i);
			UpdateBoundingBox();
		}
	}
}

const Box& Mesh::GetBoundingBox() const
{
	return mBoundingBox;
}

void Mesh::UpdateBoundingBox()
{
	mBoundingBox.Zero();

	for (int i = 0; i < mMeshChunkList.GetSize(); i++)
	{
		mMeshChunkList[i]->UpdateBoundingBox();
		mBoundingBox.AddBox(mMeshChunkList[i]->GetBoundingBox());
	}
}

void Mesh::RecompileMesh()
{
	uint count = 0;
	uint offset = 0;

	if (!mVertexBuffer)
		mVertexBuffer = GetDevice()->GetRenderer()->CreateVertexBuffer();
	if (!mIndexBuffer)
		mIndexBuffer = GetDevice()->GetRenderer()->CreateIndexBuffer();

	count = 0;
	for(int i=0; i<mMeshChunkList.GetSize(); i++)
	{
		mMeshChunkList[i]->UpdateNormals();
		count += mMeshChunkList[i]->mVertexList.GetSize();
	}

	mVertexBuffer->SetVertexData((VertexBufferMode) (VBM_NORMAL_COORDS | VBM_TEXTURE_COORDS | VBM_COLOR_COORDS), NULL, count);
	
	offset = 0;
	for(int i=0; i<mMeshChunkList.GetSize(); i++)
	{
		uint size = mMeshChunkList[i]->mVertexList.GetSize() * sizeof(VertexData);
		mVertexBuffer->SetVertexSubData((float*) mMeshChunkList[i]->mVertexList.GetBegin(), offset, mMeshChunkList[i]->mVertexList.GetSize());

		offset += size;
	}

	//Index buffer
	count = 0;
	for(int i=0; i<mMeshChunkList.GetSize(); i++)
	{
		count += mMeshChunkList[i]->mFaceList.GetSize() * 3;
	}

	mIndexBuffer->SetIndexData(NULL, count);
	
	offset = 0;
	for(int i=0; i<mMeshChunkList.GetSize(); i++)
	{
		uint size = mMeshChunkList[i]->mFaceList.GetSize() * sizeof(ushort) * 3;
		mIndexBuffer->SetIndexSubData((ushort*) mMeshChunkList[i]->mFaceList.GetBegin(), offset, mMeshChunkList[i]->mFaceList.GetSize() * 3);

		offset += size;
	}

	mGlobalVertexCount = count;

}

} // namespace Crown

