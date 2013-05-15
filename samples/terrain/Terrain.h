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

#include "Vec3.h"
#include "Triangle.h"
#include "Intersection.h"
#include "Ray.h"
#include "Renderer.h"

#define MAX_BRUSH_SIZE 256

namespace crown
{

class Terrain
{

public:

				Terrain();
				~Terrain();

	void		CreateTerrain(uint32_t xSize, uint32_t zSize, uint32_t tilePerMeter, float initialHeight);

	float		GetHeightAt(uint32_t x, uint32_t z) const;
	float		GetHeightAt(const Vec3& xyz) const;

	void		SetHeightAt(uint32_t x, uint32_t z, float height);
	void		SetHeightAt(const Vec3& xyz, float height);

	void		WorldToHeight(const Vec3& xyz, uint32_t& x, uint32_t& z) const;

	void		UpdateVertexBuffer(bool recomputeNormals);

	bool		TraceRay(const Ray& ray, Triangle& result, Triangle& tri2, real& dist);

	uint32_t	SnapToGrid(const Vec3& vertex);

	void		BuildBrush(uint32_t width, uint32_t height, float smooth);
	float		GaussDist(float x, float y, float sigma);

	void		ApplyBrush(uint32_t x, uint32_t z, float scale);
	void		ApplyBrush(const Vec3& xyz, float scale);

	void		PlotCircle(int32_t xx, int32_t yy, int32_t radius, int32_t i);

	void		Render();

private:

	uint32_t		mSizeX;				// X in meters
	uint32_t		mSizeZ;				// Z in meters
	float			mOffsetX;
	float			mOffsetZ;
	uint32_t		mTilePerMeter;		// How many tiles per linear meter?
	uint32_t		mTilesInSizeX;
	uint32_t		mTilesInSizeZ;
	uint32_t		mVerticesInSizeX;
	uint32_t		mVerticesInSizeZ;
	float*			mHeights;			// Contains the heights

	float			mMinHeight;
	float			mMaxHeight;

	uint32_t		mVertexCount;
	Vec3*			mVertices;

	uint32_t		mNormalCount;
	Vec3*			mNormals;

	uint32_t		mTexCoordCount;
	Vec2*			mTexCoords;

	uint32_t		mIndexCount;
	uint16_t*		mIndices;

	VertexBufferId	m_vertex_buffer;
	VertexBufferId	m_normal_buffer;
	VertexBufferId	m_tex_coord_buffer;
	IndexBufferId	m_index_buffer;

	uint32_t		mBrushWidth;
	uint32_t		mBrushHeight;
	float			mBrush[MAX_BRUSH_SIZE * MAX_BRUSH_SIZE];
};

} // namespace crown

