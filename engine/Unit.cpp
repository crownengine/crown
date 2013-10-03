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

#include "Unit.h"

namespace crown
{

//-----------------------------------------------------------------------------
void Unit::create(const Vec3& pos, const Quat& rot)
{
	m_root_node = m_scene_graph.create_node(pos, rot);
}

//-----------------------------------------------------------------------------
void Unit::destroy()
{

}

//-----------------------------------------------------------------------------
void Unit::load(UnitResource* ur)
{
	m_resource = ur;
}

//-----------------------------------------------------------------------------
void Unit::unload()
{
}

//-----------------------------------------------------------------------------
void Unit::reload(UnitResource* new_ur)
{
	(void)new_ur;
}

//-----------------------------------------------------------------------------
Vec3 Unit::local_position() const
{
	return m_scene_graph.local_position(m_root_node);
}

//-----------------------------------------------------------------------------
Quat Unit::local_rotation() const
{
	return m_scene_graph.local_rotation(m_root_node);
}

//-----------------------------------------------------------------------------
Mat4 Unit::local_pose() const
{
	return m_scene_graph.local_pose(m_root_node);
}

//-----------------------------------------------------------------------------
Vec3 Unit::world_position() const
{
	return m_scene_graph.world_position(m_root_node);
}

//-----------------------------------------------------------------------------
Quat Unit::world_rotation() const
{
	return m_scene_graph.world_rotation(m_root_node);
}

//-----------------------------------------------------------------------------
Mat4 Unit::world_pose() const
{
	return m_scene_graph.world_pose(m_root_node);
}

//-----------------------------------------------------------------------------
void Unit::set_local_position(const Vec3& pos)
{
	m_scene_graph.set_local_position(m_root_node, pos);
}

//-----------------------------------------------------------------------------
void Unit::set_local_rotation(const Quat& rot)
{
	m_scene_graph.set_local_rotation(m_root_node, rot);
}

//-----------------------------------------------------------------------------
void Unit::set_local_pose(const Mat4& pose)
{
	m_scene_graph.set_local_pose(m_root_node, pose);
}

} // namespace crown
