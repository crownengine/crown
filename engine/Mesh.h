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

#include "RendererTypes.h"
#include "Matrix4x4.h"

namespace crown
{

struct MeshResource;
class Vector3;
class Quaternion;

struct Mesh
{
	void			create(const MeshResource* mr, int32_t node, const Vector3& pos, const Quaternion& rot);

	Vector3			local_position() const;
	Quaternion		local_rotation() const;
	Matrix4x4		local_pose() const;

	Vector3			world_position() const;
	Quaternion		world_rotation() const;
	Matrix4x4		world_pose() const;

	void			set_local_position(const Vector3& pos);
	void			set_local_rotation(const Quaternion& rot);
	void			set_local_pose(const Matrix4x4& pose);

public:

	int32_t m_node;

	Matrix4x4 m_local_pose;
	Matrix4x4 m_world_pose;

	VertexBufferId m_vbuffer;
	IndexBufferId m_ibuffer;
};

} // namespace crown
