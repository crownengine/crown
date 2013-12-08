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

class Matrix3x3;
class Quaternion;
class Vector3;
class Vector4;

/// Column-major 4x4 matrix.
/// 
/// The engine uses column vectors for coordinate space transformations
/// so you'll have to specify transformations in reverse order.
/// e.g. (rotation * translation * vector) will produce the result of first translating
/// and then rotating the vector.
/// Also note that a column major matrix needs to be placed to the left of a
/// vector by matrix multiplication, so, to multiply a vector by a matrix you'll have
/// to write something like: matrix * vector. Since we are also using column vectors, inverting
/// the operands would result in an impossible operation.
/// 
/// @a verbatim:
///   X base vector
///     | Y base vector
///     |   | Z base vector
///     |   |   | Translation vector
///     |   |   |   |
/// 1 [ Xx  Yx  Zx  Tx ]
/// 2 | Xy  Yy  Zy  Ty |
/// 3 | Xz  Yz  Zz  Tz |
/// 4 [ 0   0   0   1  ]
///     1   2   3   4
class Matrix4x4
{

public:

	float				m[16];

	/// Does nothing for efficiency.
						Matrix4x4();	

	/// Constructs from a set of float
						Matrix4x4(float r1c1, float r2c1, float r3c1, float r4c1,
									float r1c2, float r2c2, float r3c2, float r4c2,
									float r1c3, float r2c3, float r3c3, float r4c3,
									float r1c4, float r2c4, float r3c4, float r4c4);

	/// Constructs from rotation @a r and position @a p.
						Matrix4x4(const Quaternion& r, const Vector3& p);
	
	/// Contructs from the @a v array
						Matrix4x4(const float v[16]);						
						Matrix4x4(const Matrix4x4& a);					

	/// Assignment operator (copies the data)
	Matrix4x4&			operator=(const Matrix4x4& a);					

	/// Random access by index
	float				operator[](uint32_t i) const;

	/// Random access by index					
	float&				operator[](uint32_t i);							

	float				operator()(uint32_t row, uint32_t column) const;	//!< Random access by row/column pair

	Matrix4x4			operator+(const Matrix4x4& a) const;
	Matrix4x4&			operator+=(const Matrix4x4& a);					
	Matrix4x4			operator-(const Matrix4x4& a) const;				
	Matrix4x4&			operator-=(const Matrix4x4& a);					
	Matrix4x4			operator*(float k) const;				
	Matrix4x4&			operator*=(float k);							
	Matrix4x4			operator/(float k) const;					
	Matrix4x4&			operator/=(float k);							
	Vector3				operator*(const Vector3& v) const;				
	Vector4				operator*(const Vector4& v) const;				
	Matrix4x4			operator*(const Matrix4x4& a) const;			
	Matrix4x4&			operator*=(const Matrix4x4& a);

	/// For simmetry
	friend Matrix4x4	operator*(float k, const Matrix4x4& a);			

	/// Builds a rotation matrix about the X axis of @a radians radians
	void				build_rotation_x(float radians);

	/// Builds a rotation matrix about the Y axis of "radians" radians	
	void				build_rotation_y(float radians);	

	/// Builds a rotation matrix about the Z axis of @a radians radians			
	void				build_rotation_z(float radians);		

	/// Builds a rotation matrix about an arbitrary axis of "radians" radians		
	void				build_rotation(const Vector3& n, float radians);

	/// Sets the rotation portion to the rotation described by @a rot.
	void				set_rotation(const Quaternion& rot);

	/// Sets the rotation portion to the rotation described by @a rot.
	void				set_rotation(const Matrix3x3& rot);

	/// Builds a perspetive projection matrix suited to Right-Handed coordinate systems
	void				build_projection_perspective_rh(float fovy, float aspect, float near, float far);

	/// Builds an orthographic projection matrix suited to Right-Handed coordinate systems	
	void				build_projection_ortho_rh(float left, float right, float bottom, float top, float near, float far);

	/// Builds a "Righ-Handed look-at" matrix from a position, a target, and an up vector
	void				build_look_at_rh(const Vector3& pos, const Vector3& target, const Vector3& up);

	/// Builds a "Viewpoint-Oriented billboard" matrix which can be used to make an object face a specific point in space	
	void				build_viewpoint_billboard(const Vector3& pos, const Vector3& target, const Vector3& up);	

	/// Builds a "Arbitrary-Axis billboard" matrix which can be used to make an object face a specific point in space
	void				build_axis_billboard(const Vector3& pos, const Vector3& target, const Vector3& axis);	

	Matrix4x4&			transpose();								
	Matrix4x4			get_transposed() const;						
	float				get_determinant() const;					
	Matrix4x4&			invert();									
	Matrix4x4			get_inverted() const;						

	/// Builds the identity matrix
	void				load_identity();							

	/// Returns a Vector3 containing the matrix's x base vector.
	Vector3				x() const;

	/// Returns a Vector3 containing the matrix's y base vector.
	Vector3				y() const;

	/// Returns a Vector3 containing the matrix's z base vector.
	Vector3				z() const;

	/// Sets the matrix's x base vector.
	void				set_x(const Vector3& x);

	/// Sets the matrix's y base vector.
	void				set_y(const Vector3& y);

	/// Sets the matrix's z base vector.
	void				set_z(const Vector3& z);

	/// Returns a Vector3 containing the matrix's translation portion
	Vector3				translation() const;	

	/// Fills the matrix's translation portion values contained in @a trans				
	void				set_translation(const Vector3& trans);			

	/// Returns a Vector3 containing the matrix's scale portion
	Vector3				get_scale() const;

	/// Fills the matrix's scale portion with the values contained in @a scale							
	void				set_scale(const Vector3& scale);				

	/// Returns the pointer to the matrix's data
	float*				to_float_ptr();

	/// Returns the pointer to the matrix's data
	const float*		to_float_ptr() const;

	/// Returns a 3x3 matrix according to the matrix's rotation portion
	Matrix3x3			to_matrix3x3() const;

	/// Returns a quaternion according to the matrix's rotation portion
	Quaternion			to_quaternion() const;

	static const Matrix4x4	IDENTITY;
};

} // namespace crown

