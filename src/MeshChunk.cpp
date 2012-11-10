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

#include "MeshChunk.h"
#include "Types.h"

namespace crown
{

MeshChunk::MeshChunk()
{
}

MeshChunk::~MeshChunk()
{

}

int MeshChunk::GetVertexCount() const
{
	return mVertexList.size();
}

int MeshChunk::GetFaceCount() const
{
	return mFaceList.size();
}

const Box& MeshChunk::GetBoundingBox() const
{
	return mBoundingBox;
}

void MeshChunk::UpdateBoundingBox()
{
	mBoundingBox.zero();

	for (uint i = 0; i < mVertexList.size(); i++)
	{
		mBoundingBox.add_point(mVertexList[i].position);
	}
}

void MeshChunk::UpdateNormals()
{
	for (uint i = 0; i < mFaceList.size(); i++)
	{
		Vec3 normal;
		Vec3 v1;
		Vec3 v2;

		v1 = mVertexList[mFaceList[i].vertex[0]].position - mVertexList[mFaceList[i].vertex[1]].position;
		v2 = mVertexList[mFaceList[i].vertex[2]].position - mVertexList[mFaceList[i].vertex[1]].position;
		
		normal = v2.cross(v1).normalize();

		mVertexList[mFaceList[i].vertex[0]].normal = normal;
		mVertexList[mFaceList[i].vertex[1]].normal = normal;
		mVertexList[mFaceList[i].vertex[2]].normal = normal;
	}
}

} // namespace crown

