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

#include "Assert.h"
#include "Types.h"
#include "MathUtils.h"
#include "Vec2.h"

namespace crown
{

/// 3D column vector.
class Vec3
{
public:

	float				x, y, z;

	/// Does nothing for efficiency.
						Vec3();	

	/// Initializes all the components to val								
						Vec3(float val);	

	/// Constructs from three components						
						Vec3(float nx, float ny, float nz);
						
	/// Constructs from array		
						Vec3(const float v[3]);					
						Vec3(const Vec3& a);	

	/// Random access by index
	float				operator[](uint32_t i) const;

	/// Random access by index			
	float&				operator[](uint32_t i);					

	Vec3				operator+(const Vec3& a) const;			
	Vec3&				operator+=(const Vec3& a);				
	Vec3 				operator-(const Vec3& a) const;			
	Vec3&				operator-=(const Vec3& a);				
	Vec3				operator*(float k) const;				
	Vec3&				operator*=(float k);						
	Vec3				operator/(float k) const;				
	Vec3&				operator/=(float k);

	/// Dot product						
	float				dot(const Vec3& a) const;

	/// Cross product				
	Vec3				cross(const Vec3& a) const;				

	/// For simmetry
	friend Vec3			operator*(float k, const Vec3& a);		

	bool				operator==(const Vec3& other) const;	
	bool				operator!=(const Vec3& other) const;

	/// Returns whether all the components of this vector are smaller than all of the "other" vector	
	bool				operator<(const Vec3& other) const;

	/// Returns whether all the components of this vector are greater than all of the "other" vector		
	bool				operator>(const Vec3& other) const;		

	/// Returns the vector's length
	float				length() const;	

	/// Returns the vector's squared length						
	float				squared_length() const;

	/// Sets the vector's length					
	void				set_length(float len);

	/// Normalizes the vector					
	Vec3&				normalize();

	/// Returns the normalized vector							
	Vec3				get_normalized() const;		

	/// Negates the vector (i.e. builds the inverse)			
	Vec3&				negate();

	/// Negates the vector (i.e. builds the inverse)								
	Vec3				operator-() const;						

	/// Returns the distance
	float				get_distance_to(const Vec3& a) const;	

	/// Returns the angle in radians
	float				get_angle_between(const Vec3& a) const;	

	/// Sets all components to zero
	void				zero();									

	/// Returns the pointer to the vector's data
	float*				to_float_ptr();	

	/// Returns the pointer to the vector's data						
	const float*			to_float_ptr() const;

	/// Returns a Vec2 with only x and y coordinates					
	Vec2				to_vec2() const;						

	static const Vec3	ZERO;
	static const Vec3	ONE;
	static const Vec3	XAXIS;
	static const Vec3	YAXIS;
	static const Vec3	ZAXIS;
};

//-----------------------------------------------------------------------------
inline Vec3::Vec3()
{
}

//-----------------------------------------------------------------------------
inline Vec3::Vec3(float val) : x(val), y(val), z(val)
{
}

//-----------------------------------------------------------------------------
inline Vec3::Vec3(float nx, float ny, float nz) : x(nx), y(ny), z(nz)
{
}

//-----------------------------------------------------------------------------
inline Vec3::Vec3(const float v[3]) : x(v[0]), y(v[1]), z(v[2])
{
}

//-----------------------------------------------------------------------------
inline Vec3::Vec3(const Vec3& a) : x(a.x), y(a.y), z(a.z)
{
}

//-----------------------------------------------------------------------------
inline float Vec3::operator[](uint32_t i) const
{
	CE_ASSERT(i < 3, "Index must be < 3");

	return (&x)[i];
}

//-----------------------------------------------------------------------------
inline float& Vec3::operator[](uint32_t i)
{
	CE_ASSERT(i < 3, "Index must be < 3");

	return (&x)[i];
}

//-----------------------------------------------------------------------------
inline Vec3 Vec3::operator+(const Vec3& a) const
{
	return Vec3(x + a.x, y + a.y, z + a.z);
}

//-----------------------------------------------------------------------------
inline Vec3& Vec3::operator+=(const Vec3& a)
{
	x += a.x;
	y += a.y;
	z += a.z;

	return *this;
}

//-----------------------------------------------------------------------------
inline Vec3 Vec3::operator-(const Vec3& a) const
{
	return Vec3(x - a.x, y - a.y, z - a.z);
}

//-----------------------------------------------------------------------------
inline Vec3& Vec3::operator-=(const Vec3& a)
{
	x -= a.x;
	y -= a.y;
	z -= a.z;

	return *this;
}

//-----------------------------------------------------------------------------
inline Vec3 Vec3::operator*(float k) const
{
	return Vec3(x * k, y * k, z * k);
}

//-----------------------------------------------------------------------------
inline Vec3& Vec3::operator*=(float k)
{
	x *= k;
	y *= k;
	z *= k;

	return *this;
}

//-----------------------------------------------------------------------------
inline Vec3 Vec3::operator/(float k) const
{
	CE_ASSERT(k != (float)0.0, "Division by zero");

	float inv = (float)(1.0 / k);

	return Vec3(x * inv, y * inv, z * inv);
}

//-----------------------------------------------------------------------------
inline Vec3& Vec3::operator/=(float k)
{
	CE_ASSERT(k != (float)0.0, "Division by zero");

	float inv = (float)(1.0 / k);

	x *= inv;
	y *= inv;
	z *= inv;

	return *this;
}

//-----------------------------------------------------------------------------
inline float Vec3::dot(const Vec3& a) const
{
	return x * a.x + y * a.y + z * a.z;
}

//-----------------------------------------------------------------------------
inline Vec3 Vec3::cross(const Vec3& a) const
{
	return Vec3(y * a.z - z * a.y, z * a.x - x * a.z, x * a.y - y * a.x);
}

//-----------------------------------------------------------------------------
inline Vec3 operator*(float k, const Vec3& a)
{
	return a * k;
}

//-----------------------------------------------------------------------------
inline bool Vec3::operator==(const Vec3& other) const
{
	return math::equals(x, other.x) && math::equals(y, other.y) && math::equals(z, other.z);
}

//-----------------------------------------------------------------------------
inline bool Vec3::operator!=(const Vec3& other) const
{
	return !math::equals(x, other.x) || !math::equals(y, other.y) || !math::equals(z, other.z);
}

//-----------------------------------------------------------------------------
inline bool Vec3::operator<(const Vec3& other) const
{
	return ((x < other.x) && (y < other.y) && (z < other.z));
}

//-----------------------------------------------------------------------------
inline bool Vec3::operator>(const Vec3& other) const
{
	return ((x > other.x) && (y > other.y) && (z > other.z));
}

//-----------------------------------------------------------------------------
inline float Vec3::length() const
{
	return math::sqrt(x * x + y * y + z * z);
}

//-----------------------------------------------------------------------------
inline float Vec3::squared_length() const
{
	return x * x + y * y + z * z;
}

//-----------------------------------------------------------------------------
inline void Vec3::set_length(float len)
{
	normalize();

	x *= len;
	y *= len;
	z *= len;
}

//-----------------------------------------------------------------------------
inline Vec3& Vec3::normalize()
{
	float len = length();

	if (math::equals(len, (float)0.0))
	{
		return *this;
	}

	len = (float)(1.0 / len); 

	x *= len;
	y *= len;
	z *= len;

	return *this;
}

//-----------------------------------------------------------------------------
inline Vec3 Vec3::get_normalized() const
{
	Vec3 tmp(x, y, z);

	return tmp.normalize();
}

//-----------------------------------------------------------------------------
inline Vec3& Vec3::negate()
{
	x = -x;
	y = -y;
	z = -z;

	return *this;
}

//-----------------------------------------------------------------------------
inline Vec3 Vec3::operator-() const
{
	return Vec3(-x, -y, -z);
}

//-----------------------------------------------------------------------------
inline float Vec3::get_distance_to(const Vec3& a) const
{
	return (*this - a).length();
}

//-----------------------------------------------------------------------------
inline float Vec3::get_angle_between(const Vec3& a) const
{
	return math::acos(this->dot(a) / (this->length() * a.length()));
}

//-----------------------------------------------------------------------------
inline void Vec3::zero()
{
	x = 0.0;
	y = 0.0;
	z = 0.0;
}

//-----------------------------------------------------------------------------
inline float* Vec3::to_float_ptr()
{
	return &x;
}

//-----------------------------------------------------------------------------
inline const float* Vec3::to_float_ptr() const
{
	return &x;
}

//-----------------------------------------------------------------------------
inline Vec2 Vec3::to_vec2() const
{
	return Vec2(x, y);
}

//-----------------------------------------------------------------------------
/// Returns the parallel portion of "v" projected onto "n"
inline Vec3 get_projected_parallel(const Vec3& v, const Vec3& n)
{
	float n_len_q;
	n_len_q = n.length();
	n_len_q = n_len_q * n_len_q;

	return n * (v.dot(n) / n_len_q);
}

//-----------------------------------------------------------------------------
/// Returns the perpendicular portion of "v" projected onto "n"
inline Vec3 get_projected_perpendicular(const Vec3& v, const Vec3& n)
{
	return v - get_projected_parallel(v, n);
}

} // namespace crown

