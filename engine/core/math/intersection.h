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

#include "math_types.h"
#include "sphere.h"

namespace crown
{

/// Returns the distance along ray (from, dir) to intersection point with plane @a p
/// or -1.0 if no intersection.
float ray_plane_intersection(const Vector3& from, const Vector3& dir, const Plane& p);

/// Returns the distance along ray (from, dir) to intersection point with sphere @a s
/// or -1.0 if no intersection.
float ray_sphere_intersection(const Vector3& from, const Vector3& dir, const Sphere& s);

/// Returns the distance along ray (from, dir) to intersection point with @a obb
/// or -1.0 if no intersection.
float ray_oobb_intersection(const Vector3& from, const Vector3& dir, const OBB& obb);

bool plane_3_intersection(const Plane& p1, const Plane& p2, const Plane& p3, Vector3& ip);
bool frustum_sphere_intersection(const Frustum& f, const Sphere& s);
bool frustum_box_intersection(const Frustum& f, const AABB& b);

} // namespace crown
