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
#include "Mat4.h"

namespace crown
{

struct MeshResource;

struct Mesh
{
	void			create(const MeshResource* mr);

	Vec3			local_position() const;
	Quat			local_rotation() const;
	Mat4			local_pose() const;

	Vec3			world_position() const;
	Quat			world_rotation() const;
	Mat4			world_pose() const;

	void			set_local_position(const Vec3& pos);
	void			set_local_rotation(const Quat& rot);
	void			set_local_pose(const Mat4& pose);

public:

	int32_t m_node;

	Mat4 m_local_pose;
	Mat4 m_world_pose;

	VertexBufferId m_vbuffer;
	IndexBufferId m_ibuffer;
};

} // namespace crown
