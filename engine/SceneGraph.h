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
#include "Mat4.h"
#include "List.h"

namespace crown
{

struct SceneGraph
{
					SceneGraph();
	int32_t			create_node(int32_t parent, const Vec3& pos, const Quat& rot);
	void			destroy_node(int32_t id);

	void			link(int32_t child, int32_t parent);
	void			unlink(int32_t child);

	void			set_local_position(int32_t node, const Vec3& pos);
	void			set_local_rotation(int32_t node, const Quat& rot);
	void			set_local_pose(int32_t node, const Mat4& pose);

	Vec3			local_position(int32_t node) const;
	Quat			local_rotation(int32_t node) const;
	Mat4			local_pose(int32_t node) const;

	Vec3			world_position(int32_t node) const;
	Quat			world_rotation(int32_t node) const;
	Mat4			world_pose(int32_t node) const;

	void			update();

public:

	List<Mat4>		m_world_poses;
	List<Mat4>		m_local_poses;
	List<int32_t>	m_parents;
};

} // namespace crown
