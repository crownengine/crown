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

#include "types.h"
#include "math_types.h"
#include "lua.hpp"

namespace crown
{

/// Global lua-related functions
namespace lua_globals
{
	/// Initializes the lua system.
	/// This is the place where to create and initialize per-application objects.
	void init();

	/// It should reverse the actions performed by lua_globals::init().
	void shutdown();

	lua_State* state();

	/// Clears temporary objects (Vector3, Matrix4x4, ...).
	void clear_temporaries();

	/// Returns a new temporary Vector2, Vector3, Matrix4x4 or Quaternion
	Vector3* next_vector3(const Vector3& v);
	Matrix4x4* next_matrix4x4(const Matrix4x4& m);
	Quaternion* next_quaternion(const Quaternion& q);

	/// Returns whether the object at stack @a index is a Vector3, Matrix4x4 or Quaternion
	bool is_vector3(int32_t index);
	bool is_matrix4x4(int32_t index);
	bool is_quaternion(int32_t index);
} // namespace lua_globals
} // namespace crown
