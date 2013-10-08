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

namespace crown
{

/// 4D column vector.
class Vector4
{
public:

	float				x, y, z, w;

	/// Does nothing for efficiency.
						Vector4();	

	/// Initializes all the components to val						
						Vector4(float val);

	/// Constructs from four components								
						Vector4(float nx, float ny, float nz, float nw);

	/// Constructs from array	
						Vector4(const float v[4]);						
						Vector4(const Vector4& a);

	/// Random access by index
	float				operator[](uint32_t i) const;	

	/// Random access by index
	float&				operator[](uint32_t i);						

	Vector4				operator+(const Vector4& a) const;				
	Vector4&			operator+=(const Vector4& a);					
	Vector4 			operator-(const Vector4& a) const;				
	Vector4&			operator-=(const Vector4& a);					
	Vector4				operator*(float k) const;					
	Vector4&			operator*=(float k);							
	Vector4				operator/(float k) const;					
	Vector4&			operator/=(float k);

	/// Dot product							
	float				dot(const Vector4& a) const;					

	/// For simmetry
	friend Vector4		operator*(float k, const Vector4& a);			

	bool				operator==(const Vector4& other) const;		
	bool				operator!=(const Vector4& other) const;

	/// Returns whether all the components of this vector are smaller than all of the @a other vector	
	bool				operator<(const Vector4& other) const;	

	/// Returns whether all the components of this vector are greater than all of the @a other vector		
	bool				operator>(const Vector4& other) const;			

	/// Returns the vector's length
	float				length() const;	

	/// Returns the vector's squared length							
	float				squared_length() const;

	/// Sets the vector's length						
	void				set_length(float len);

	/// Normalizes the vector						
	Vector4&			normalize();

	/// Returns the normalized vector								
	Vector4				get_normalized() const;

	/// Negates the vector (i.e. builds the inverse)						
	Vector4&			negate();	

	/// Negates the vector (i.e. builds the inverse)								
	Vector4				operator-() const;							

	/// Returns the distance
	float				get_distance_to(const Vector4& a) const;

	/// Returns the angle in radians		
	float				get_angle_between(const Vector4& a) const;		

	/// Sets all components to zero
	void				zero();										

	/// Returns the pointer to the vector's data
	float*				to_float_ptr();	

	/// Returns the pointer to the vector's data							
	const float*		to_float_ptr() const;						

	static const Vector4	ZERO;
	static const Vector4	ONE;
	static const Vector4	XAXIS;
	static const Vector4	YAXIS;
	static const Vector4	ZAXIS;
	static const Vector4	WAXIS;
};

//-----------------------------------------------------------------------------
inline Vector4::Vector4()
{
}

//-----------------------------------------------------------------------------
inline Vector4::Vector4(float val) : x(val), y(val), z(val), w(val)
{
}

//-----------------------------------------------------------------------------
inline Vector4::Vector4(float nx, float ny, float nz, float nw) : x(nx), y(ny), z(nz), w(nw)
{
}

//-----------------------------------------------------------------------------
inline Vector4::Vector4(const float a[4]) : x(a[0]), y(a[1]), z(a[2]), w(a[3])
{
}

//-----------------------------------------------------------------------------
inline Vector4::Vector4(const Vector4& a) : x(a.x), y(a.y), z(a.z), w(a.w)
{
}

//-----------------------------------------------------------------------------
inline float Vector4::operator[](uint32_t i) const
{
	CE_ASSERT(i < 4, "Index must be < 4");

	return (&x)[i];
}

//-----------------------------------------------------------------------------
inline float& Vector4::operator[](uint32_t i)
{
	CE_ASSERT(i < 4, "Index must be < 4");

	return (&x)[i];
}

//-----------------------------------------------------------------------------
inline Vector4 Vector4::operator+(const Vector4& a) const
{
	return Vector4(x + a.x, y + a.y, z + a.z, w + a.w);
}

//-----------------------------------------------------------------------------
inline Vector4& Vector4::operator+=(const Vector4& a)
{
	x += a.x;
	y += a.y;
	z += a.z;
	w += a.w;

	return *this;
}

//-----------------------------------------------------------------------------
inline Vector4 Vector4::operator-(const Vector4& a) const
{
	return Vector4(x - a.x, y - a.y, z - a.z, w - a.w);
}

//-----------------------------------------------------------------------------
inline Vector4& Vector4::operator-=(const Vector4& a)
{
	x -= a.x;
	y -= a.y;
	z -= a.z;
	w -= a.w;

	return *this;
}

//-----------------------------------------------------------------------------
inline Vector4 Vector4::operator*(float k) const
{
	return Vector4(x * k, y * k, z * k, w * k);
}

//-----------------------------------------------------------------------------
inline Vector4& Vector4::operator*=(float k)
{
	x *= k;
	y *= k;
	z *= k;
	w *= k;

	return *this;
}

//-----------------------------------------------------------------------------
inline Vector4 Vector4::operator/(float k) const
{
	CE_ASSERT(k != (float)0.0, "Division by zero");

	float inv = (float)(1.0 / k);

	return Vector4(x * inv, y * inv, z * inv, w * inv);
}

//-----------------------------------------------------------------------------
inline Vector4& Vector4::operator/=(float k)
{
	CE_ASSERT(k != (float)0.0, "Division by zero");

	float inv = (float)(1.0 / k);

	x *= inv;
	y *= inv;
	z *= inv;
	w *= inv;

	return *this;
}

//-----------------------------------------------------------------------------
inline float Vector4::dot(const Vector4& a) const
{
	return x * a.x + y * a.y + z * a.z + w * a.w;
}

//-----------------------------------------------------------------------------
inline Vector4 operator*(float k, const Vector4& a)
{
	return a * k;
}

//-----------------------------------------------------------------------------
inline bool Vector4::operator==(const Vector4& other) const
{
	return math::equals(x, other.x) && math::equals(y, other.y) && math::equals(z, other.z) && math::equals(w, other.w);
}

//-----------------------------------------------------------------------------
inline bool Vector4::operator!=(const Vector4& other) const
{
	return !math::equals(x, other.x) || !math::equals(y, other.y) || !math::equals(z, other.z) || !math::equals(w, other.w);
}

//-----------------------------------------------------------------------------
inline bool Vector4::operator<(const Vector4& other) const
{
	return ((x < other.x) && (y < other.y) && (z < other.z) && (w < other.w));
}

//-----------------------------------------------------------------------------
inline bool Vector4::operator>(const Vector4& other) const
{
	return ((x > other.x) && (y > other.y) && (z > other.z) && (w > other.w));
}

//-----------------------------------------------------------------------------
inline float Vector4::length() const
{
	return math::sqrt(x * x + y * y + z * z + w * w);
}

//-----------------------------------------------------------------------------
inline float Vector4::squared_length() const
{
	return x * x + y * y + z * z + w * w;
}

//-----------------------------------------------------------------------------
inline Vector4& Vector4::normalize()
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
	w *= len;

	return *this;
}

//-----------------------------------------------------------------------------
inline Vector4 Vector4::get_normalized() const
{
	Vector4 tmp(x, y, z, w);

	return tmp.normalize();
}

//-----------------------------------------------------------------------------
inline Vector4& Vector4::negate()
{
	x = -x;
	y = -y;
	z = -z;
	w = -w;

	return *this;
}

//-----------------------------------------------------------------------------
inline Vector4 Vector4::operator-() const
{
	return Vector4(-x, -y, -z, -w);
}

//-----------------------------------------------------------------------------
inline float Vector4::get_distance_to(const Vector4& a) const
{
	return (*this - a).length();
}

//-----------------------------------------------------------------------------
inline float Vector4::get_angle_between(const Vector4& a) const
{
	return math::acos(this->dot(a) / (this->length() * a.length()));
}

//-----------------------------------------------------------------------------
inline void Vector4::zero()
{
	x = 0.0;
	y = 0.0;
	z = 0.0;
	w = 0.0;
}

//-----------------------------------------------------------------------------
inline float* Vector4::to_float_ptr()
{
	return &x;
}

//-----------------------------------------------------------------------------
inline const float* Vector4::to_float_ptr() const
{
	return &x;
}

} // namespace crown

