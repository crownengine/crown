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
#include "Vec3.h"

namespace crown
{

class Angles;
class Mat3;
class Mat4;

/**
	Quaternion.

@Note:
	This implementation uses the standard quaternion
	multiplication equation, so, the order of multiplication
	for multiple rotations is in a reverse fashion:
	p' = qpq^-1 where p is the point and q the rotation quaternion

	p' = (ba)p(ba)^-1 where p is the point and (ba) the concatenation of two successive rotations
	In this case, the point p is first rotated by the quaternion a and then by the quaternion b.
	The transformation order is reversed.
*/
class Quat
{
public:

	Vec3		v;
	real		w;

				Quat();								//!< Constructor
				Quat(real angle, const Vec3& v);	//!< Builds the quaternion from an angle and a vector
				~Quat();							//!< Destructor

	void		negate();							//!< Negates the quaternion
	void		load_identity();					//!< Builds the identity quaternion
	real		length() const;						//!< Returns the quaternion's length
	void		conjugate();						//!< Conjugates the quaternion
	Quat		get_conjugate() const;				//!< Returns the quaternion's conjugate
	Quat		get_inverse() const;				//!< Quaternion's inverse

	Angles		to_angles() const;
	Mat3		to_mat3() const;
	Mat4		to_mat4() const;

	Quat		operator*(const Quat& b) const;		//!< Cross product
	Quat		operator*(const real& k) const;		//!< Multiplication by a scalar

	Quat		power(real exp);

	Str			to_str() const;						//!< Returns a Str containing the quaternion's components
};

} // namespace crown

