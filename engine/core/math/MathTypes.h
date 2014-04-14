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

#include "Types.h"

namespace crown
{

/// @defgroup Math Math

/// @ingroup Math
struct Vector2
{
	Vector2();
	Vector2(float val);
	Vector2(float nx, float ny);
	Vector2(const float v[2]);

	float operator[](uint32_t i) const;
	float& operator[](uint32_t i);

	Vector2& operator+=(const Vector2& a);
	Vector2& operator-=(const Vector2& a);
	Vector2& operator*=(float k);
	Vector2& operator/=(float k);

	float x, y;
};

/// @ingroup Math
struct Vector3
{
	Vector3();
	Vector3(float val);
	Vector3(float nx, float ny, float nz);
	Vector3(const float v[3]);

	float operator[](uint32_t i) const;
	float& operator[](uint32_t i);

	Vector3& operator+=(const Vector3& a);
	Vector3& operator-=(const Vector3& a);
	Vector3& operator*=(float k);
	Vector3& operator/=(float k);

	float x, y, z;
};

/// @ingroup Math
struct Vector4
{
	Vector4();
	Vector4(const Vector3& a, float w);
	Vector4(float val);
	Vector4(float nx, float ny, float nz, float nw);
	Vector4(const float v[3]);

	float operator[](uint32_t i) const;
	float& operator[](uint32_t i);

	Vector4& operator+=(const Vector4& a);
	Vector4& operator-=(const Vector4& a);
	Vector4& operator*=(float k);
	Vector4& operator/=(float k);

	float x, y, z, w;
};

/// @ingroup Math
struct Quaternion
{
	Quaternion();
	Quaternion(float nx, float ny, float nz, float nw);
	Quaternion(const Vector3& axis, float angle);

	Quaternion& operator*=(const Quaternion& a);

	float x, y, z, w;
};

/// @ingroup Math
struct Matrix3x3
{
	Matrix3x3();
	Matrix3x3(const Vector3& x, const Vector3& y, const Vector3& z);
	Matrix3x3(const Quaternion& r);

	Matrix3x3(float r1c1, float r2c1, float r3c1, float r1c2, float r2c2, float r3c2,
				float r1c3, float r2c3, float r3c3);

	Matrix3x3(const float v[9]);

	float& operator[](uint32_t i);
	const float& operator[](uint32_t i) const;

	Matrix3x3& operator+=(const Matrix3x3& a);
	Matrix3x3& operator-=(const Matrix3x3& a);
	Matrix3x3& operator*=(const Matrix3x3& a);
	Matrix3x3& operator*=(float k);
	Matrix3x3& operator/=(float k);

	Vector3 x, y, z;
};

/// @ingroup Math
struct Matrix4x4
{
	Matrix4x4();
	Matrix4x4(const Vector3& x, const Vector3& y, const Vector3& z, const Vector3& t);
	Matrix4x4(const Quaternion& r, const Vector3& p);
	Matrix4x4(const Matrix3x3& m);

	Matrix4x4(float r1c1, float r2c1, float r3c1, float r4c1,
				float r1c2, float r2c2, float r3c2, float r4c2,
				float r1c3, float r2c3, float r3c3, float r4c3,
				float r1c4, float r2c4, float r3c4, float r4c4);

	Matrix4x4(const float v[16]);

	float& operator[](uint32_t i);
	const float& operator[](uint32_t i) const;

	Matrix4x4& operator+=(const Matrix4x4& a);
	Matrix4x4& operator-=(const Matrix4x4& a);
	Matrix4x4& operator*=(const Matrix4x4& a);
	Matrix4x4& operator*=(float k);
	Matrix4x4& operator/=(float k);

	Vector4 x, y, z, t;
};

/// @ingroup Math
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
///
/// @ingroup Math
struct Plane
{
	/// Does nothing for efficiency.
	Plane();						
	Plane(const Vector3& n, float d);

	Vector3 n;
	float d;
};

/// @ingroup Math
struct Frustum
{
	Frustum();

	Plane left;
	Plane right;
	Plane bottom;
	Plane top;
	Plane near;
	Plane far;
};

} // namespace crown
