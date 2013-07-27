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

class Mat4;
class Quat;
class Vec3;

/// Column major 3x3 matrix.
/// 
/// The engine uses column vectors for coordinate space transformations
/// so you'll have to specify transformations in reverse order.
/// e.g. (rotation translation vector) will produce the result of first translating
/// and then rotating the vector.
/// Also note that a column major matrix needs to be placed to the left of a
/// vector by matrix multiplication, so, to multiply a vector by a matrix you'll have
/// to write something like: matrix vector. Since we are also using column vectors, inverting
/// the operands would result in an impossible operation.
/// 
/// @a verbatim:
///   X base vector
///     | Y base vector
///     |   | Z base vector
///     |   |   |
/// 1 [ Xx  Yx  Zx ]
/// 2 | Xy  Yy  Zy |
/// 3 [ Xz  Yz  Zz ]
///     1   2   3
class Mat3
{

public:

	float				m[9];

	/// Does nothing for efficiency.
						Mat3();			

	/// Constructs from a set of float
						Mat3(float r1c1, float r2c1, float r3c1, float r1c2, float r2c2, float r3c2, float r1c3, float r2c3, float r3c3);
	
	/// Constructs from the @a v array
						Mat3(const float v[9]);						
						Mat3(const Mat3& a);	

	/// Assignment operator (copies the data)
	Mat3&				operator=(const Mat3& a);					

	/// Random access by index
	float				operator[](uint32_t i) const;		

	/// Random access by index			
	float&				operator[](uint32_t i);							

	/// Random access by row/column pair
	float				operator()(uint32_t row, uint32_t column) const;	

	Mat3				operator+(const Mat3& a) const;				
	Mat3&				operator+=(const Mat3& a);					
	Mat3				operator-(const Mat3& a) const;				
	Mat3&				operator-=(const Mat3& a);					
	Mat3				operator*(float k) const;					
	Mat3&				operator*=(float k);						
	Mat3				operator/(float k) const;					
	Mat3&				operator/=(float k);							
	Vec3				operator*(const Vec3& v) const;			
	Mat3				operator*(const Mat3& a) const;				
	Mat3&				operator*=(const Mat3& a);			

	/// For simmetry
	friend Mat3			operator*(float k, const Mat3& a);			

	/// Builds a rotation matrix about the X axis of @a radians radians
	void				build_rotation_x(float radians);			

	/// Builds a rotation matrix about the Y axis of @a radians radians	
	void				build_rotation_y(float radians);	

	/// Builds a rotation matrix about the Z axis of @a radians radians			
	void				build_rotation_z(float radians);	

	/// Builds a rotation matrix about an arbitrary axis of "radians" radians			
	void				build_rotation(const Vec3& n, float radians);

	Mat3&				transpose();								
	Mat3				get_transposed() const;						
	float				get_determinant() const;					
	Mat3&				invert();									
	Mat3				get_inverted() const;						

	/// Builds the identity matrix
	void				load_identity();							

	/// Returns a Vec3 containing the matrix's x base vector.
	Vec3				x() const;

	/// Returns a Vec3 containing the matrix's y base vector.
	Vec3				y() const;

	/// Returns a Vec3 containing the matrix's z base vector.
	Vec3				z() const;

	/// Sets the matrix's x base vector.
	void				set_x(const Vec3& x);

	/// Sets the matrix's y base vector.
	void				set_y(const Vec3& y);

	/// Sets the matrix's z base vector.
	void				set_z(const Vec3& z);

	/// Returns a Vec3 containing the matrix's scale portion
	Vec3				get_scale() const;	

	/// Fills the matrix's scale portion with the values contained in @a scale				
	void				set_scale(const Vec3& scale);				

	/// Returns the pointer to the matrix's data
	float*				to_float_ptr();				

	/// Returns the pointer to the matrix's data				
	const float*		to_float_ptr() const;

	/// Returns a 4x4 matrix according to the matrix's rotation portion						
	Mat4				to_mat4() const;

	/// Returns a quaternion according to the matrix's rotation portion							
	Quat				to_quat() const;							

	static const Mat3	IDENTITY;
};

} // namespace crown

