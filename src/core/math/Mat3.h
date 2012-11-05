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

class Angles;
class Mat4;
class Quat;
class Str;
class Vec3;

/**
	Column major 3x3 matrix.

	The engine uses column vectors for coordinate space transformations
	so you'll have to specify transformations in reverse order.
	e.g. (rotation translation vector) will produce the result of first translating
	and then rotating the vector.
	Also note that a column major matrix needs to be placed to the left of a
	vector by matrix multiplication, so, to multiply a vector by a matrix you'll have
	to write something like: matrix vector. Since we are also using column vectors, inverting
	the operands would result in an impossible operation.

@verbatim:
	  X base vector
		| Y base vector
		|   | Z base vector
		|   |   |
	1 [ Xx  Yx  Zx ]
	2 | Xy  Yy  Zy |
	3 [ Xz  Yz  Zz ]
		1   2   3
*/
class Mat3
{

public:

	real				m[9];

						Mat3();										//!< Constructor, does nothing for efficency
						//! Constructs from a set of real
						Mat3(real r1c1, real r2c1, real r3c1, real r1c2, real r2c2, real r3c2, real r1c3, real r2c3, real r3c3);
						Mat3(const real v[9]);						//!< Constructs from the "v" array
						Mat3(const Mat3& a);						//!< Copy constructor
						~Mat3();									//!< Destructor

	Mat3&				operator=(const Mat3& a);					//!< Assignment operator (copies the data)

	real				operator[](uint i) const;					//!< Random access by index
	real&				operator[](uint i);							//!< Random access by index

	real				operator()(uint row, uint column) const;	//!< Random access by row/column pair

	Mat3				operator+(const Mat3& a) const;				//!< Addition
	Mat3&				operator+=(const Mat3& a);					//!< Addition
	Mat3				operator-(const Mat3& a) const;				//!< Subtraction
	Mat3&				operator-=(const Mat3& a);					//!< Subtraction
	Mat3				operator*(real k) const;					//!< Multiplication by scalar
	Mat3&				operator*=(real k);							//!< Multiplication by scalar
	Mat3				operator/(real k) const;					//!< Division by scalar
	Mat3&				operator/=(real k);							//!< Division by scalar
	Vec3				operator*(const Vec3& v) const;				//!< Multiplication by vector
	Mat3				operator*(const Mat3& a) const;				//!< Multiplication
	Mat3&				operator*=(const Mat3& a);					//!< Multiplication

	friend Mat3			operator*(real k, const Mat3& a);			//!< For simmetry

	void				build_rotation_x(real radians);				//!< Builds a rotation matrix about the X axis of "radians" radians
	void				build_rotation_y(real radians);				//!< Builds a rotation matrix about the Y axis of "radians" radians
	void				build_rotation_z(real radians);				//!< Builds a rotation matrix about the Z axis of "radians" radians
	void				build_rotation(const Vec3& n, real radians);//!< Builds a rotation matrix about an arbitrary axis of "radians" radians

	Mat3&				transpose();								//!< Transposes the matrix
	Mat3				get_transposed() const;						//!< Returns the transposed of the matrix
	real				get_determinant() const;					//!< Returns the matrix's determinant
	Mat3&				invert();									//!< Builds the inverse of the matrix
	Mat3				get_inverted() const;						//!< Returns the inverse of the matrix

	void				load_identity();							//!< Builds the identity matrix

	Vec3				get_scale() const;							//!< Returns a Vec3 containing the matrix's scale portion
	void				set_scale(const Vec3& scale);				//!< Fills the matrix's scale portion with the values contained in "scale"

	real*				to_float_ptr();								//!< Returns the pointer to the matrix's data
	const real*			to_float_ptr() const;						//!< Returns the pointer to the matrix's data
	Str					to_str() const;								//!< Returns a Str containing the matrix's components
	Angles				to_angles() const;							//!< Returns an angles according to the matrix's rotation portion
	Mat4				to_mat4() const;							//!< Returns a 4x4 matrix according to the matrix's rotation portion
	Quat				to_quat() const;							//!< Returns a quaternion according to the matrix's rotation portion

	static const Mat3	IDENTITY;
};

} // namespace crown

