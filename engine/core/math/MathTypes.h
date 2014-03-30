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

namespace crown
{

struct Vector2
{
	Vector2();
	Vector2(float val);
	Vector2(float nx, float ny);
	Vector2(const float v[2]);
	Vector2(const Vector2& a);

	float operator[](uint32_t i) const;
	float& operator[](uint32_t i);

	Vector2& operator+=(const Vector2& a);
	Vector2& operator-=(const Vector2& a);
	Vector2& operator*=(float k);
	Vector2& operator/=(float k);

	float x, y;
};

struct Vector3
{
	Vector3();
	Vector3(float val);
	Vector3(float nx, float ny, float nz);
	Vector3(const float v[3]);
	Vector3(const Vector3& a);

	float operator[](uint32_t i) const;
	float& operator[](uint32_t i);

	Vector3& operator+=(const Vector3& a);
	Vector3& operator-=(const Vector3& a);
	Vector3& operator*=(float k);
	Vector3& operator/=(float k);

	float x, y, z;
};

struct Vector4
{
	Vector4();
	Vector4(float val);
	Vector4(float nx, float ny, float nz, float nw);
	Vector4(const float v[3]);
	Vector4(const Vector4& a);

	float operator[](uint32_t i) const;
	float& operator[](uint32_t i);

	Vector4& operator+=(const Vector4& a);
	Vector4& operator-=(const Vector4& a);
	Vector4& operator*=(float k);
	Vector4& operator/=(float k);

	float x, y, z, w;
};

struct Quaternion
{
	Quaternion();
	Quaternion(float nx, float ny, float nz, float nw);
	Quaternion(const Vector3& axis, float angle);

	Quaternion& operator*=(const Quaternion& a);

	float x, y, z, w;
};

struct AABB
{
	AABB();

	/// Constructs from @a min and @a max.
	AABB(const Vector3& min, const Vector3& max);

	Vector3 min;
	Vector3 max;
};

/// 3D Plane.
/// The form is ax + by + cz + d = 0
/// where: d = -vector3::dot(n, p)
struct Plane
{
	/// Does nothing for efficiency.
	Plane();						
	Plane(const Vector3& n, float d);
	Plane(const Plane& p);

	Vector3 n;
	float d;
};

struct Frustum
{
	Frustum();
	Frustum(const Frustum& f);

	Plane left;
	Plane right;
	Plane bottom;
	Plane top;
	Plane near;
	Plane far;
};

} // namespace crown
