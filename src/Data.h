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
#include "Vec2.h"
#include "Vec3.h"
#include "Color4.h"

namespace crown
{

struct VertexData
{
	VertexData(const Vec3& position = Vec3::ZERO, const Vec3& normal = Vec3::ZERO,
		const Vec2& uv = Vec2::ZERO, const Color4& color = Color4::WHITE)
	{
		this->position = position;
		this->normal = normal;
		this->uv = uv;
		this->color = color;
	}

	bool operator==(const VertexData& other) const
	{
		return position == other.position && normal == other.normal && uv == other.uv && color == other.color;
	}

	Vec3 position;
	Vec3 normal;
	Vec2 uv;
	Color4 color;
};

struct FaceData
{
	FaceData() {}

	FaceData(uint16_t v1, uint16_t v2, uint16_t v3)
	{
		vertex[0] = (v1);
		vertex[1] = (v2);
		vertex[2] = (v3);
	}

	bool operator==(const FaceData& other) const
	{
		return vertex[0] == other.vertex[0] && vertex[1] == other.vertex[1] && vertex[2] == other.vertex[2];
	}

	uint16_t vertex[3];
};

} // namespace crown

