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

#include "MeshManager.h"
#include "Log.h"
#include "MathUtils.h"

namespace crown
{

MeshManager::MeshManager()
{
}

Mesh* MeshManager::Create(const char* name, bool& created)
{
	Mesh* mesh = static_cast<Mesh*>(ResourceManager::Create(name, created));

	return mesh;
}

Mesh* MeshManager::Load(const char* /*name*/)
{
	Log::E("MeshManager::Load: Not implemented yet.");
	return 0;
}

Mesh* MeshManager::LoadCube(const char* name, float size)
{
	bool created;
	Mesh* mesh = Create(name, created);

	if (mesh != NULL && created)
	{
		MeshChunk* chunk = new MeshChunk();

		float actual = math::abs(size) * 0.5f;

		/**

		         19------18
		         /      / 
		       16------17

		    15    6------7   10       
		    /|    |      |   /|
		  14 |  3------2 | 11 |
		   |12  | 5----|-4  | 9
		   |/   |      |    |/
		  13    0------1    8

		         20------21
		         /      /
		       23------22

		*/

		// 6 faces, with pos, normal and uv
		// Face ================================================================================================== Index
		// Front
		chunk->mVertexList.push_back(VertexData(Vec3(-actual, -actual,  actual),  Vec3::ZAXIS, Vec2(0.0f, 0.0f))); // 0
		chunk->mVertexList.push_back(VertexData(Vec3( actual, -actual,  actual),  Vec3::ZAXIS, Vec2(1.0f, 0.0f))); // 1
		chunk->mVertexList.push_back(VertexData(Vec3( actual,  actual,  actual),  Vec3::ZAXIS, Vec2(1.0f, 1.0f))); // 2
		chunk->mVertexList.push_back(VertexData(Vec3(-actual,  actual,  actual),  Vec3::ZAXIS, Vec2(0.0f, 1.0f))); // 3

		// Back
		chunk->mVertexList.push_back(VertexData(Vec3( actual, -actual, -actual), -Vec3::ZAXIS, Vec2(0.0f, 0.0f))); // 4
		chunk->mVertexList.push_back(VertexData(Vec3(-actual, -actual, -actual), -Vec3::ZAXIS, Vec2(1.0f, 0.0f))); // 5
		chunk->mVertexList.push_back(VertexData(Vec3(-actual,  actual, -actual), -Vec3::ZAXIS, Vec2(1.0f, 1.0f))); // 6
		chunk->mVertexList.push_back(VertexData(Vec3( actual,  actual, -actual), -Vec3::ZAXIS, Vec2(0.0f, 1.0f))); // 7

		// Right
		chunk->mVertexList.push_back(VertexData(Vec3( actual, -actual,  actual),  Vec3::XAXIS, Vec2(0.0f, 0.0f))); // 8
		chunk->mVertexList.push_back(VertexData(Vec3( actual, -actual, -actual),  Vec3::XAXIS, Vec2(1.0f, 0.0f))); // 9
		chunk->mVertexList.push_back(VertexData(Vec3( actual,  actual, -actual),  Vec3::XAXIS, Vec2(1.0f, 1.0f))); // 10
		chunk->mVertexList.push_back(VertexData(Vec3( actual,  actual,  actual),  Vec3::XAXIS, Vec2(0.0f, 1.0f))); // 11

		// Left
		chunk->mVertexList.push_back(VertexData(Vec3(-actual, -actual, -actual), -Vec3::XAXIS, Vec2(0.0f, 0.0f))); // 12
		chunk->mVertexList.push_back(VertexData(Vec3(-actual, -actual,  actual), -Vec3::XAXIS, Vec2(1.0f, 0.0f))); // 13
		chunk->mVertexList.push_back(VertexData(Vec3(-actual,  actual,  actual), -Vec3::XAXIS, Vec2(1.0f, 1.0f))); // 14
		chunk->mVertexList.push_back(VertexData(Vec3(-actual,  actual, -actual), -Vec3::XAXIS, Vec2(0.0f, 1.0f))); // 15

		// Top
		chunk->mVertexList.push_back(VertexData(Vec3(-actual,  actual,  actual),  Vec3::YAXIS, Vec2(0.0f, 0.0f))); // 16
		chunk->mVertexList.push_back(VertexData(Vec3( actual,  actual,  actual),  Vec3::YAXIS, Vec2(1.0f, 0.0f))); // 17
		chunk->mVertexList.push_back(VertexData(Vec3( actual,  actual, -actual),  Vec3::YAXIS, Vec2(1.0f, 1.0f))); // 18
		chunk->mVertexList.push_back(VertexData(Vec3(-actual,  actual, -actual),  Vec3::YAXIS, Vec2(0.0f, 1.0f))); // 19

		// Bottom
		chunk->mVertexList.push_back(VertexData(Vec3(-actual, -actual, -actual), -Vec3::YAXIS, Vec2(0.0f, 0.0f))); // 20
		chunk->mVertexList.push_back(VertexData(Vec3( actual, -actual, -actual), -Vec3::YAXIS, Vec2(1.0f, 0.0f))); // 21
		chunk->mVertexList.push_back(VertexData(Vec3( actual, -actual,  actual), -Vec3::YAXIS, Vec2(1.0f, 1.0f))); // 22
		chunk->mVertexList.push_back(VertexData(Vec3(-actual, -actual,  actual), -Vec3::YAXIS, Vec2(0.0f, 1.0f))); // 23

		// 12 triangles, CCW order
		chunk->mFaceList.push_back(FaceData(0, 1, 2));
		chunk->mFaceList.push_back(FaceData(0, 2, 3));

		chunk->mFaceList.push_back(FaceData(4, 5, 6));
		chunk->mFaceList.push_back(FaceData(4, 6, 7));

		chunk->mFaceList.push_back(FaceData(8, 9, 10));
		chunk->mFaceList.push_back(FaceData(8, 10, 11));

		chunk->mFaceList.push_back(FaceData(12, 13, 14));
		chunk->mFaceList.push_back(FaceData(12, 14, 15));

		chunk->mFaceList.push_back(FaceData(16, 17, 18));
		chunk->mFaceList.push_back(FaceData(16, 18, 19));

		chunk->mFaceList.push_back(FaceData(20, 21, 22));
		chunk->mFaceList.push_back(FaceData(20, 22, 23));

		mesh->AddMeshChunk(chunk);
		mesh->UpdateBoundingBox();
		mesh->RecompileMesh();
	}

	return mesh;
}

Mesh* MeshManager::LoadPlane(const char* name, float width, float height)
{
	bool created;
	Mesh* mesh = Create(name, created);

	if (mesh != NULL && created)
	{
		MeshChunk* chunk = new MeshChunk();

		float actualWidth = math::abs(width) * 0.5f;
		float actualHeight = math::abs(height) * 0.5f;

		/**

			    (-x; +y)      (+x; +y)
		          ______________
		         /             /
		        /     ^ (+z)  /
		       /      |      /
		      /             /
		     /_____________/

		(-x; -y)      (+x; -y)

		*/

		// Only a face, with pos, normal and UVs
		chunk->mVertexList.push_back(VertexData(Vec3(-actualWidth, -actualHeight,  0.0f),  Vec3::ZAXIS, Vec2(0.0f, 0.0f)));
		chunk->mVertexList.push_back(VertexData(Vec3( actualWidth, -actualHeight,  0.0f),  Vec3::ZAXIS, Vec2(1.0f, 0.0f)));
		chunk->mVertexList.push_back(VertexData(Vec3( actualWidth,  actualHeight,  0.0f),  Vec3::ZAXIS, Vec2(1.0f, 1.0f)));
		chunk->mVertexList.push_back(VertexData(Vec3(-actualWidth,  actualHeight,  0.0f),  Vec3::ZAXIS, Vec2(0.0f, 1.0f)));

		// 2 triangles, CCW order
		chunk->mFaceList.push_back(FaceData(0, 1, 2));
		chunk->mFaceList.push_back(FaceData(0, 2, 3));

		mesh->AddMeshChunk(chunk);
		mesh->UpdateBoundingBox();
		mesh->RecompileMesh();
	}

	return mesh;
}

Mesh* MeshManager::LoadGrid(const char* name, uint size, float tileSize)
{
	if (size < 1)
	{
		Log::E("MeshManager::LoadGrid: Size must be > 0");
		return 0;
	}

	bool created;
	Mesh* mesh = Create(name, created);

	if (mesh != NULL && created)
	{
		MeshChunk* chunk = new MeshChunk();

		float actual = ((float)size * tileSize * 0.5f);

		/**

			   (-x; -z)    (+x; -z)
		         ____________
		        /__/__/__/__/
		       /__/__/__/__/  ^ (+y)
		      /__/__/__/__/   |
		     /__/__/__/__/

		(-x; +z)    (+x; +z)

		*/

		// Populate vertex list (generate a grid lying on the xz-plane and facing upwards)
		float vCoord = 0.0f;//(float)size;
		float zPos = actual;
		for (uint h = 0; h <= size; h++)
		{
			float uCoord = 0.0f;
			float xPos = -actual;
			for (uint w = 0; w <= size; w++)
			{
				chunk->mVertexList.push_back(VertexData(Vec3(xPos, 0.0f, zPos), Vec3::YAXIS, Vec2(uCoord, vCoord)));
				xPos += tileSize;
				uCoord += 1.0f;// / ((float)size + 1.0f);
			}
			zPos -= tileSize;
			vCoord += 1.0f;// / ((float)size + 1.0f);
		}

		// Generate faces
		for (uint h = 0; h < size; h++)
		{
			for (uint w = 0; w < size; w++)
			{
				uint firstRow = (h * (size + 1)) + w;
				uint secondRow = ((h + 1) * (size + 1)) + w;
				chunk->mFaceList.push_back(FaceData(firstRow, firstRow + 1, secondRow + 1));
				chunk->mFaceList.push_back(FaceData(firstRow, secondRow + 1, secondRow));
			}
		}

		mesh->AddMeshChunk(chunk);
		mesh->UpdateBoundingBox();
		mesh->RecompileMesh();
	}

	return mesh;
}

Mesh* MeshManager::CreateSpecific(const char* name)
{
	return new Mesh();
}

MeshManager meshMgr;
MeshManager* GetMeshManager()
{
	return &meshMgr;
}

} // namespace crown

