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
#include "Vector2.h"

namespace crown
{

/// 3D column vector.
struct Vector3
{
public:

	float				x, y, z;

	/// Does nothing for efficiency.
						Vector3();	

	/// Initializes all the components to val								
						Vector3(float val);	

	/// Constructs from three components						
						Vector3(float nx, float ny, float nz);
						
	/// Constructs from array		
						Vector3(const float v[3]);					
						Vector3(const Vector3& a);	

	/// Random access by index
	float				operator[](uint32_t i) const;

	/// Random access by index			
	float&				operator[](uint32_t i);					

	Vector3				operator+(const Vector3& a) const;			
	Vector3&			operator+=(const Vector3& a);				
	Vector3 			operator-(const Vector3& a) const;			
	Vector3&			operator-=(const Vector3& a);				
	Vector3				operator*(float k) const;				
	Vector3&			operator*=(float k);						
	Vector3				operator/(float k) const;				
	Vector3&			operator/=(float k);

	/// Dot product						
	float				dot(const Vector3& a) const;

	/// Cross product				
	Vector3				cross(const Vector3& a) const;				

	/// For simmetry
	friend Vector3		operator*(float k, const Vector3& a);		

	bool				operator==(const Vector3& other) const;	
	bool				operator!=(const Vector3& other) const;

	/// Returns whether all the components of this vector are smaller than all of the "other" vector	
	bool				operator<(const Vector3& other) const;

	/// Returns whether all the components of this vector are greater than all of the "other" vector		
	bool				operator>(const Vector3& other) const;		

	/// Returns the vector's length
	float				length() const;	

	/// Returns the vector's squared length						
	float				squared_length() const;

	/// Sets the vector's length					
	void				set_length(float len);

	/// Normalizes the vector					
	Vector3&			normalize();

	/// Returns the normalized vector							
	Vector3				get_normalized() const;		

	/// Negates the vector (i.e. builds the inverse)			
	Vector3&			negate();

	/// Negates the vector (i.e. builds the inverse)								
	Vector3				operator-() const;						

	/// Returns the distance
	float				get_distance_to(const Vector3& a) const;	

	/// Returns the angle in radians
	float				get_angle_between(const Vector3& a) const;	

	/// Sets all components to zero
	void				zero();									

	/// Returns the pointer to the vector's data
	float*				to_float_ptr();	

	/// Returns the pointer to the vector's data						
	const float*		to_float_ptr() const;

	/// Returns a Vector2 with only x and y coordinates					
	Vector2				to_vec2() const;						

	static const Vector3	ZERO;
	static const Vector3	ONE;
	static const Vector3	XAXIS;
	static const Vector3	YAXIS;
	static const Vector3	ZAXIS;
};

//-----------------------------------------------------------------------------
inline Vector3::Vector3()
{
}

//-----------------------------------------------------------------------------
inline Vector3::Vector3(float val) : x(val), y(val), z(val)
{
}

//-----------------------------------------------------------------------------
inline Vector3::Vector3(float nx, float ny, float nz) : x(nx), y(ny), z(nz)
{
}

//-----------------------------------------------------------------------------
inline Vector3::Vector3(const float v[3]) : x(v[0]), y(v[1]), z(v[2])
{
}

//-----------------------------------------------------------------------------
inline Vector3::Vector3(const Vector3& a) : x(a.x), y(a.y), z(a.z)
{
}

//-----------------------------------------------------------------------------
inline float Vector3::operator[](uint32_t i) const
{
	CE_ASSERT(i < 3, "Index must be < 3");

	return (&x)[i];
}

//-----------------------------------------------------------------------------
inline float& Vector3::operator[](uint32_t i)
{
	CE_ASSERT(i < 3, "Index must be < 3");

	return (&x)[i];
}

//-----------------------------------------------------------------------------
inline Vector3 Vector3::operator+(const Vector3& a) const
{
	return Vector3(x + a.x, y + a.y, z + a.z);
}

//-----------------------------------------------------------------------------
inline Vector3& Vector3::operator+=(const Vector3& a)
{
	x += a.x;
	y += a.y;
	z += a.z;

	return *this;
}

//-----------------------------------------------------------------------------
inline Vector3 Vector3::operator-(const Vector3& a) const
{
	return Vector3(x - a.x, y - a.y, z - a.z);
}

//-----------------------------------------------------------------------------
inline Vector3& Vector3::operator-=(const Vector3& a)
{
	x -= a.x;
	y -= a.y;
	z -= a.z;

	return *this;
}

//-----------------------------------------------------------------------------
inline Vector3 Vector3::operator*(float k) const
{
	return Vector3(x * k, y * k, z * k);
}

//-----------------------------------------------------------------------------
inline Vector3& Vector3::operator*=(float k)
{
	x *= k;
	y *= k;
	z *= k;

	return *this;
}

//-----------------------------------------------------------------------------
inline Vector3 Vector3::operator/(float k) const
{
	CE_ASSERT(k != (float)0.0, "Division by zero");

	float inv = (float)(1.0 / k);

	return Vector3(x * inv, y * inv, z * inv);
}

//-----------------------------------------------------------------------------
inline Vector3& Vector3::operator/=(float k)
{
	CE_ASSERT(k != (float)0.0, "Division by zero");

	float inv = (float)(1.0 / k);

	x *= inv;
	y *= inv;
	z *= inv;

	return *this;
}

//-----------------------------------------------------------------------------
inline float Vector3::dot(const Vector3& a) const
{
	return x * a.x + y * a.y + z * a.z;
}

//-----------------------------------------------------------------------------
inline Vector3 Vector3::cross(const Vector3& a) const
{
	return Vector3(y * a.z - z * a.y, z * a.x - x * a.z, x * a.y - y * a.x);
}

//-----------------------------------------------------------------------------
inline Vector3 operator*(float k, const Vector3& a)
{
	return a * k;
}

//-----------------------------------------------------------------------------
inline bool Vector3::operator==(const Vector3& other) const
{
	return math::equals(x, other.x) && math::equals(y, other.y) && math::equals(z, other.z);
}

//-----------------------------------------------------------------------------
inline bool Vector3::operator!=(const Vector3& other) const
{
	return !math::equals(x, other.x) || !math::equals(y, other.y) || !math::equals(z, other.z);
}

//-----------------------------------------------------------------------------
inline bool Vector3::operator<(const Vector3& other) const
{
	return ((x < other.x) && (y < other.y) && (z < other.z));
}

//-----------------------------------------------------------------------------
inline bool Vector3::operator>(const Vector3& other) const
{
	return ((x > other.x) && (y > other.y) && (z > other.z));
}

//-----------------------------------------------------------------------------
inline float Vector3::length() const
{
	return math::sqrt(x * x + y * y + z * z);
}

//-----------------------------------------------------------------------------
inline float Vector3::squared_length() const
{
	return x * x + y * y + z * z;
}

//-----------------------------------------------------------------------------
inline void Vector3::set_length(float len)
{
	normalize();

	x *= len;
	y *= len;
	z *= len;
}

//-----------------------------------------------------------------------------
inline Vector3& Vector3::normalize()
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
inline Vector3 Vector3::get_normalized() const
{
	Vector3 tmp(x, y, z);

	return tmp.normalize();
}

//-----------------------------------------------------------------------------
inline Vector3& Vector3::negate()
{
	x = -x;
	y = -y;
	z = -z;

	return *this;
}

//-----------------------------------------------------------------------------
inline Vector3 Vector3::operator-() const
{
	return Vector3(-x, -y, -z);
}

//-----------------------------------------------------------------------------
inline float Vector3::get_distance_to(const Vector3& a) const
{
	return (*this - a).length();
}

//-----------------------------------------------------------------------------
inline float Vector3::get_angle_between(const Vector3& a) const
{
	return math::acos(this->dot(a) / (this->length() * a.length()));
}

//-----------------------------------------------------------------------------
inline void Vector3::zero()
{
	x = 0.0;
	y = 0.0;
	z = 0.0;
}

//-----------------------------------------------------------------------------
inline float* Vector3::to_float_ptr()
{
	return &x;
}

//-----------------------------------------------------------------------------
inline const float* Vector3::to_float_ptr() const
{
	return &x;
}

//-----------------------------------------------------------------------------
inline Vector2 Vector3::to_vec2() const
{
	return Vector2(x, y);
}

//-----------------------------------------------------------------------------
/// Returns the parallel portion of "v" projected onto "n"
inline Vector3 get_projected_parallel(const Vector3& v, const Vector3& n)
{
	float n_len_q;
	n_len_q = n.length();
	n_len_q = n_len_q * n_len_q;

	return n * (v.dot(n) / n_len_q);
}

//-----------------------------------------------------------------------------
/// Returns the perpendicular portion of "v" projected onto "n"
inline Vector3 get_projected_perpendicular(const Vector3& v, const Vector3& n)
{
	return v - get_projected_parallel(v, n);
}

} // namespace crown

