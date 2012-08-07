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

#include "Mesh.h"
#include "Vec3.h"
#include "Triangle.h"
#include "Intersection.h"
#include "Ray.h"

#define MAX_BRUSH_SIZE 256

namespace Crown
{

class Terrain
{

public:

				Terrain();
				~Terrain();

	void		CreateTerrain(uint xSize, uint zSize, uint tilePerMeter, float initialHeight);

	float		GetHeightAt(uint x, uint z) const;
	float		GetHeightAt(const Vec3& xyz) const;

	void		SetHeightAt(uint x, uint z, float height);
	void		SetHeightAt(const Vec3& xyz, float height);

	void		WorldToHeight(const Vec3& xyz, uint& x, uint& z) const;

	void		UpdateVertexBuffer(bool recomputeNormals);

	bool		TraceRay(const Ray& ray, Triangle& result, Triangle& tri2, real& dist);

	uint		SnapToGrid(const Vec3& vertex);

	void		SaveAsBmp(const char* name);

	void		BuildBrush(uint width, uint height, float smooth);
	float		GaussDist(float x, float y, float sigma);

	void		ApplyBrush(uint x, uint z, float scale);
	void		ApplyBrush(const Vec3& xyz, float scale);

	void		PlotCircle(int xx, int yy, int radius, int i);

	void		Render();

private:

	uint		mSizeX;				// X in meters
	uint		mSizeZ;				// Z in meters
	float		mOffsetX;
	float		mOffsetZ;
	uint		mTilePerMeter;		// How many tiles per linear meter?
	uint		mTilesInSizeX;
	uint		mTilesInSizeZ;
	uint		mVerticesInSizeX;
	uint		mVerticesInSizeZ;
	float*		mHeights;			// Contains the heights

	float		mMinHeight;
	float		mMaxHeight;

	uint		mVertexCount;
	Vec3*		mVertices;

	uint		mNormalCount;
	Vec3*		mNormals;

	uint		mTexCoordCount;
	Vec2*		mTexCoords;

	uint		mIndexCount;
	ushort*		mIndices;

	int			mBrushWidth;
	int			mBrushHeight;
	float		mBrush[MAX_BRUSH_SIZE * MAX_BRUSH_SIZE];
};

} // namespace Crown

