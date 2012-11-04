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

namespace Crown
{

class Mat3;
class Mat4;
class Quat;
class Str;

/**
	Euler angles.

	Used to describe the orientation.
	All values are expressed in radians.
*/
class Angles
{

public:

	real					heading, pitch, bank;

							Angles();						//!< Constructor, does nothing for efficiency
							~Angles();						//!< Destructor
							Angles(real h, real p, real b);	//!< Construct from three values


	Str						to_str() const;					//!< Returns a Str containing the angles' components
	Mat3					to_mat3() const;					//!< Returns an equivalent Mat3 representation.
	Mat4					to_mat4() const;					//!< Returns an equivalent Mat4 representation.
	Quat					to_quat() const;					//!< Returns an equivalent Quat representation.

	static const Angles		ZERO;
};

} // namespace Crown

