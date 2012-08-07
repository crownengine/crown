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

#include "Plane.h"
#include "Sphere.h"
#include "Box.h"
#include "Ray.h"

namespace Crown
{

enum ShapeType
{
	ST_PLANE				= BIT(0),
	ST_SPHERE				= BIT(1),
	ST_BOX					= BIT(2),
	ST_RAY					= BIT(3),
	ST_CONVEX				= BIT(4)
//	ST_RECT					= BIT(4),
//	ST_CIRCLE				= BIT(5),
//	ST_ORIENTED_RECTANGLE	= BIT(6),
//	ST_RAY_2D				= BIT(7),
//	ST_SEGMENT				= BIT(8)
};

class Shape
{
public:
					Shape(ShapeType type);
					~Shape();

	ShapeType		GetShapeType();

private:

	ShapeType		mType;
	void*			mShape;
};

}

