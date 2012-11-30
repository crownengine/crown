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

namespace crown
{

class Mat3;
class Quat;
class Vec3;
class Vec4;

/**
	Column major 4x4 matrix.

	The engine uses column vectors for coordinate space transformations
	so you'll have to specify transformations in reverse order.
	e.g. (rotation * translation * vector) will produce the result of first translating
	and then rotating the vector.
	Also note that a column major matrix needs to be placed to the left of a
	vector by matrix multiplication, so, to multiply a vector by a matrix you'll have
	to write something like: matrix * vector. Since we are also using column vectors, inverting
	the operands would result in an impossible operation.

@verbatim:
	  X base vector
		| Y base vector
		|   | Z base vector
		|   |   | Translation vector
		|   |   |   |
	1 [ Xx  Yx  Zx  Tx ]
	2 | Xy  Yy  Zy  Ty |
	3 | Xz  Yz  Zz  Tz |
	4 [ 0   0   0   1  ]
		1   2   3   4
*/
class Mat4
{

public:

	real				m[16];

						Mat4();										//!< Constructor, does nothing for efficiency
						//! Constructs from a set of real
						Mat4(real r1c1, real r2c1, real r3c1, real r4c1, real r1c2, real r2c2, real r3c2, real r4c2, real r1c3, real r2c3, real r3c3, real r4c3, real r1c4, real r2c4, real r3c4, real r4c4);
						Mat4(const real v[16]);						//!< Contructs from the "v" array
						Mat4(const Mat4& a);						//!< Copy constructor
						~Mat4();									//!< Destructor

	Mat4&				operator=(const Mat4& a);					//!< Assignment operator (copies the data)

	real				operator[](uint32_t i) const;					//!< Random access by index
	real&				operator[](uint32_t i);							//!< Random access by index

	real				operator()(uint32_t row, uint32_t column) const;	//!< Random access by row/column pair

	Mat4				operator+(const Mat4& a) const;				//!< Addition
	Mat4&				operator+=(const Mat4& a);					//!< Addition
	Mat4				operator-(const Mat4& a) const;				//!< Subtraction
	Mat4&				operator-=(const Mat4& a);					//!< Subtraction
	Mat4				operator*(real k) const;					//!< Multiplication by scalar
	Mat4&				operator*=(real k);							//!< Multiplication by scalar
	Mat4				operator/(real k) const;					//!< Division by scalar
	Mat4&				operator/=(real k);							//!< Division by scalar
	Vec3				operator*(const Vec3& v) const;				//!< Multiplication by vector
	Vec4				operator*(const Vec4& v) const;				//!< Multiplication by vector
	Mat4				operator*(const Mat4& a) const;				//!< Multiplication
	Mat4&				operator*=(const Mat4& a);					//!< Multiplication

	friend Mat4			operator*(real k, const Mat4& a);			//!< For simmetry

	void				build_rotation_x(real radians);				//!< Builds a rotation matrix about the X axis of "radians" radians
	void				build_rotation_y(real radians);				//!< Builds a rotation matrix about the Y axis of "radians" radians
	void				build_rotation_z(real radians);				//!< Builds a rotation matrix about the Z axis of "radians" radians
	void				build_rotation(const Vec3& n, real radians);//!< Builds a rotation matrix about an arbitrary axis of "radians" radians
	void				build_projection_perspective_rh(real fovy, real aspect, real near, real far);	//!< Builds a perspetive projection matrix suited to Right-Handed coordinate systems
	void				build_projection_perspective_lh(real fovy, real aspect, real near, real far);	//!< Builds a perspective projection matrix suited to Left-Handed coordinate systems
	void				build_projection_ortho_rh(real width, real height, real near, real far);		//!< Builds an orthographic projection matrix suited to Right-Handed coordinate systems
	void				build_projection_ortho_lh(real width, real height, real near, real far);		//!< Builds an orthographic projection matrix suited to Left-Handed coordinate systems
	void				build_projection_ortho_2d_rh(real width, real height, real near, real far);	//!< Builds a 2d orthographic projection matrix suited to Right-Handed coordinate systems

	void				build_look_at_rh(const Vec3& pos, const Vec3& target, const Vec3& up);	//!< Builds a "Righ-Handed look-at" matrix from a position, a target, and an up vector
	void				build_look_at_lh(const Vec3& pos, const Vec3& target, const Vec3& up);	//!< Builds a "Left-Handed look-at" matrix from a position, a target, and an up vector
	void				build_viewpoint32_t_billboard(const Vec3& pos, const Vec3& target, const Vec3& up);	//!< Builds a "Viewpoint32_t-Oriented billboard" matrix which can be used to make an object face a specific point32_t in space
	void				build_axis_billboard(const Vec3& pos, const Vec3& target, const Vec3& axis);	//!< Builds a "Arbitrary-Axis billboard" matrix which can be used to make an object face a specific point32_t in space

	Mat4&				transpose();								//!< Transposes the matrix
	Mat4				get_transposed() const;						//!< Returns the transposed of the matrix
	real				get_determinant() const;					//!< Returns the matrix's determinant
	Mat4&				invert();									//!< Inverts the matrix
	Mat4				get_inverted() const;						//!< Returns the inverse of the matrix

	void				load_identity();							//!< Builds the identity matrix

	Vec3				get_translation() const;					//!< Returns a Vec3 containing the matrix's translation portion
	void				set_translation(const Vec3& trans);			//!< Fills the matrix's translation portion values contained in "trans"

	Vec3				get_scale() const;							//!< Returns a Vec3 containing the matrix's scale portion
	void				set_scale(const Vec3& scale);				//!< Fills the matrix's scale portion with the values contained in "scale"

	real*				to_float_ptr();								//!< Returns the point32_ter to the matrix's data
	const real*			to_float_ptr() const;						//!< Returns the point32_ter to the matrix's data
	Mat3				to_mat3() const;							//!< Returns a 3x3 matrix according to the matrix's rotation portion
	Quat				to_quat() const;							//!< Returns a quaternion according to the matrix's rotation portion

	static const Mat4	IDENTITY;
};

} // namespace crown

