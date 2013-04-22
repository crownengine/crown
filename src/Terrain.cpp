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

#include "Terrain.h"
#include "Device.h"
#include "Renderer.h"
#include "MathUtils.h"
#include "Log.h"
#include "Vec2.h"
#include "Interpolation.h"

namespace crown
{

Terrain::Terrain() :
	mHeights(NULL),
	mMinHeight(-10.0f),
	mMaxHeight(10.0f),
	mVertices(NULL),
	mNormals(NULL),
	mTexCoords(NULL),
	mIndices(NULL)
{

}

Terrain::~Terrain()
{
	if (mHeights != NULL)
	{
		delete[] mHeights;
	}

	delete[] mVertices;
	delete[] mNormals;
	delete[] mTexCoords;
	delete[] mIndices;
}

void Terrain::CreateTerrain(uint32_t xSize, uint32_t zSize, uint32_t tilePerMeter, float initialHeight)
{
	assert(xSize > 0);
	assert(zSize > 0);
	assert(tilePerMeter > 0);

	mSizeX = xSize;
	mSizeZ = zSize;
	mTilePerMeter = tilePerMeter;

	float tileStep	= +(float)mSizeX / ((float)mSizeX * (float)mTilePerMeter);	// Tile step is the same for both x and z ;)

	mTilesInSizeX = ((float)mSizeX / tileStep);
	mTilesInSizeZ = ((float)mSizeZ / tileStep);
	mVerticesInSizeX = mTilesInSizeX + 1;
	mVerticesInSizeZ = mTilesInSizeZ + 1;

	Log::D("Vertices in size x/z: %d %d\n", mVerticesInSizeX, mVerticesInSizeZ);

	uint32_t heightsCount = mVerticesInSizeX * mVerticesInSizeZ;

	mHeights = new float[heightsCount];

	// Init heights
	for (uint32_t i = 0; i < heightsCount; i++)
	{
		mHeights[i] = initialHeight;
	}

	// Construct drawing data
	mVertices = new Vec3[heightsCount];			// There are as many vertices as heights
	mVertexCount = heightsCount;
	mNormals = new Vec3[heightsCount];			// Same as vertices
	mNormalCount = heightsCount;
	mTexCoords = new Vec2[heightsCount];		// Same as vertices
	mTexCoordCount = heightsCount;
	mIndices = new uint16_t[mTilesInSizeX * mTilesInSizeZ * 6];	//
	mIndexCount = mTilesInSizeX * mTilesInSizeZ * 6;

	// Populate vertex list (generate a grid lying on the xz-plane and facing upwards)
	float xStart	= -(float)mSizeX * 0.5f;
	float zStart	= +(float)mSizeZ * 0.5f;

	mOffsetX = xStart;
	mOffsetZ = zStart;

	uint32_t vIndex = 0;			// Just because I'm lazy
	float xCurrent;				// Keeps track of current x position
	float zCurrent = zStart;	// Keeps track of current z position
	for (uint32_t z = 0; z < mVerticesInSizeZ; z++)
	{
		xCurrent = xStart;	
		for (uint32_t x = 0; x < mVerticesInSizeX; x++)
		{
			mVertices[vIndex].x = xCurrent;
			mVertices[vIndex].y = mHeights[vIndex];
			mVertices[vIndex].z = zCurrent;

			mNormals[vIndex].x = 0.0f;
			mNormals[vIndex].y = 1.0f;
			mNormals[vIndex].z = 0.0f;

			mTexCoords[vIndex].x = (float)x;
			mTexCoords[vIndex].y = (float)z;

			vIndex++;
			xCurrent += tileStep;
		}
		zCurrent -= tileStep;
	}

	// Populate index list
	uint32_t iIndex = 0;
	for (uint32_t z = 0; z < mTilesInSizeZ; z++)
	{
		for (uint32_t x = 0; x < mTilesInSizeX; x++)
		{
			uint32_t firstRow = z * mVerticesInSizeX + x;
			uint32_t secondRow = (z + 1) * mVerticesInSizeX + x;

			mIndices[iIndex + 0] = firstRow;
			mIndices[iIndex + 1] = secondRow + 1;
			mIndices[iIndex + 2] = secondRow;
			mIndices[iIndex + 3] = firstRow;
			mIndices[iIndex + 4] = firstRow + 1;
			mIndices[iIndex + 5] = secondRow + 1;
			iIndex += 6;
		}
	}
}

void Terrain::UpdateVertexBuffer(bool recomputeNormals)
{
	uint32_t vIndex = 0;
	for (uint32_t z = 0; z < mVerticesInSizeZ; z++)
	{
		for (uint32_t x = 0; x < mVerticesInSizeX; x++)
		{
			mVertices[vIndex].y = mHeights[vIndex];
			vIndex++;
		}
	}

	if (recomputeNormals)
	{
		for (uint32_t i = 0; i < mIndexCount; i += 3)
		{
			Vec3 normal;
			Vec3 v1;
			Vec3 v2;

			v1 = mVertices[mIndices[i + 0]] - mVertices[mIndices[i + 1]];
			v2 = mVertices[mIndices[i + 2]] - mVertices[mIndices[i + 1]];
		
			normal = v2.cross(v1).normalize();

			mNormals[mIndices[i + 0]] = normal;
			mNormals[mIndices[i + 1]] = normal;
			mNormals[mIndices[i + 2]] = normal;
		}
	}
}

float Terrain::GetHeightAt(uint32_t x, uint32_t z) const
{
	if (x > mVerticesInSizeX) return 0.0f;
	if (z > mVerticesInSizeZ) return 0.0f;

	return mHeights[z * mVerticesInSizeX + x];
}

float Terrain::GetHeightAt(const Vec3& xyz) const
{
	uint32_t x, z;

	WorldToHeight(xyz, x, z);

	return GetHeightAt(x, z);
}

void Terrain::SetHeightAt(uint32_t x, uint32_t z, float height)
{
	if (x >= mVerticesInSizeX) return;
	if (z >= mVerticesInSizeZ) return;

	mHeights[z * mVerticesInSizeX + x] += height;
	mHeights[z * mVerticesInSizeX + x] = math::clamp_to_range(mMinHeight, mMaxHeight, mHeights[z * mVerticesInSizeX + x]);
}

void Terrain::SetHeightAt(const Vec3& xyz, float height)
{
	uint32_t x, z;

	WorldToHeight(xyz, x, z);

	SetHeightAt(x + 0, z + 0, height);
}

void Terrain::WorldToHeight(const Vec3& xyz, uint32_t& x, uint32_t& z) const
{
	Vec3 offsetted = xyz + Vec3(-mOffsetX, 0.0f, mOffsetZ);
	offsetted.z = (float)mSizeZ - offsetted.z;

	x = (uint32_t)offsetted.x;
	z = (uint32_t)offsetted.z;
}

bool Terrain::TraceRay(const Ray& ray, Triangle& result, Triangle& /*tri2*/, real& dist)
{
	bool hit = false;
	real minDist = 9999999.0f;

	for (uint32_t i = 0; i < mIndexCount; i += 3)
	{
		Triangle tri;

		tri.v1 = mVertices[mIndices[i + 0]];
		tri.v2 = mVertices[mIndices[i + 1]];
		tri.v3 = mVertices[mIndices[i + 2]];

		real ret;
		Vec3 int32_tersectionPoint32_t;
		if (Intersection::TestRayTriangle(ray, tri, ret, int32_tersectionPoint32_t))
		{
			if (ret < minDist)
			{
				minDist = ret;
				result = tri;
			}

			hit = true;
		}
	}

	dist = minDist;
	return hit;
}

uint32_t Terrain::SnapToGrid(const Vec3& vertex)
{
	float minDist = 9999999.0f;
	uint32_t indexToSnapped;
	// Find the snapped point32_t to input vertex
	for (uint32_t i = 0; i < mVertexCount; i++)
	{
		Vec3 tmp = mVertices[i];
		Vec3 vertex2 = vertex;
		tmp.y = vertex2.y = 0.0f;

		if (tmp.get_distance_to(vertex2) < minDist)
		{
			indexToSnapped = i;
			minDist = tmp.get_distance_to(vertex2);
		}
	}

	return indexToSnapped;
}

void Terrain::Render()
{
	Renderer* renderer = GetDevice()->GetRenderer();

	renderer->render_triangles(
				mVertices[0].to_float_ptr(),
				mNormals[0].to_float_ptr(),
				mTexCoords[0].to_float_ptr(),
				mIndices,
				mTilesInSizeX * mTilesInSizeZ * 6);
}

float Terrain::GaussDist(float x, float y, float sigma)
{
	float gauss =  1.0f / math::TWO_PI * (sigma * sigma);
	float e = 2.71828183f;

	float exponent = ((x * x) + (y * y)) / (2.0f * (sigma * sigma));

	return gauss * pow(e, -exponent);
}

void Terrain::BuildBrush(uint32_t width, uint32_t height, float smooth)
{
	assert(width < MAX_BRUSH_SIZE);
	assert(height < MAX_BRUSH_SIZE);

	mBrushWidth = width;
	mBrushHeight = height;

	float xStart = -(float)width * 0.5f;
	float yStart = -(float)height * 0.5f;

	float xCurrent = xStart;
	for (uint32_t i = 0; i <= width; i++)
	{
		float yCurrent = yStart;
		for (uint32_t j = 0; j <= height; j++)
		{
			mBrush[j * MAX_BRUSH_SIZE + i] = GaussDist(xCurrent, yCurrent, smooth);
			yCurrent += 1.0f;
		}
		xCurrent += 1.0f;
	}
}

void Terrain::PlotCircle(int32_t xx, int32_t yy, int32_t radius, int32_t i)
{
	for (int32_t j = 0; j < 256 * 256; j++)
	{
		mBrush[j] = 0;
	}

  int32_t x, y;

	mBrushWidth = radius * 2;
	mBrushHeight = radius * 2;

  for (y = -radius; y <= radius; y++)
    for (x = -radius; x <= radius; x++)
      if ((x * x) + (y * y) <= (radius * radius))
		{
			float rDist = 1.0 - math::sqrt(x * x + y * y) / radius;

			if (i == 0)
			{
				mBrush[(y + yy) * MAX_BRUSH_SIZE + (x + xx)] = Interpolation::linear(0.0f, 1.0f, rDist);
			}
			else if (i == 1)
			{
				mBrush[(y + yy) * MAX_BRUSH_SIZE + (x + xx)] = Interpolation::cosine(0.0f, 1.0f, rDist);
			}
			else if (i == 2)
			{
				mBrush[(y + yy) * MAX_BRUSH_SIZE + (x + xx)] = Interpolation::cubic(0.0f, 1.0f, rDist);
			}
		}
}

void Terrain::ApplyBrush(uint32_t x, uint32_t z, float scale)
{
	uint32_t offsetX = mBrushWidth / 2;
	uint32_t offsetY = mBrushHeight / 2;
	for (uint32_t i = 0; i < mBrushWidth; i++)
	{
		for (uint32_t j = 0; j < mBrushHeight; j++)
		{
			SetHeightAt((x - offsetX) + i, (z - offsetY) + j, scale * mBrush[j * MAX_BRUSH_SIZE + i]);
		}
	}
}

void Terrain::ApplyBrush(const Vec3& xyz, float scale)
{
	uint32_t x, z;

	WorldToHeight(xyz, x, z);

	ApplyBrush(x, z, scale);
}

} // namespace crown

