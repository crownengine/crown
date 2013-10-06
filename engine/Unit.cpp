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
#include "IdTable.h"
#include "World.h"

namespace crown
{

typedef Id CameraId;

//-----------------------------------------------------------------------------
void Unit::create(World& creator, const Vec3& pos, const Quat& rot)
{
	m_creator = &creator;
	m_root_node = m_scene_graph.create_node(-1, pos, rot);

	int32_t camera_node = m_scene_graph.create_node(m_root_node, Vec3::ZERO, Quat::IDENTITY);
	CameraId camera = m_creator->create_camera(*this, camera_node);
	m_camera = m_creator->lookup_camera(camera);
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
Vec3 Unit::local_position(int32_t node) const
{
	return m_scene_graph.local_position(node);
}

//-----------------------------------------------------------------------------
Quat Unit::local_rotation(int32_t node) const
{
	return m_scene_graph.local_rotation(node);
}

//-----------------------------------------------------------------------------
Mat4 Unit::local_pose(int32_t node) const
{
	return m_scene_graph.local_pose(node);
}

//-----------------------------------------------------------------------------
Vec3 Unit::world_position(int32_t node) const
{
	return m_scene_graph.world_position(node);
}

//-----------------------------------------------------------------------------
Quat Unit::world_rotation(int32_t node) const
{
	return m_scene_graph.world_rotation(node);
}

//-----------------------------------------------------------------------------
Mat4 Unit::world_pose(int32_t node) const
{
	return m_scene_graph.world_pose(node);
}

//-----------------------------------------------------------------------------
void Unit::set_local_position(const Vec3& pos, int32_t node)
{
	m_scene_graph.set_local_position(node, pos);
}

//-----------------------------------------------------------------------------
void Unit::set_local_rotation(const Quat& rot, int32_t node)
{
	m_scene_graph.set_local_rotation(node, rot);
}

//-----------------------------------------------------------------------------
void Unit::set_local_pose(const Mat4& pose, int32_t node)
{
	m_scene_graph.set_local_pose(node, pose);
}

//-----------------------------------------------------------------------------
Camera* Unit::camera(const char* /*name*/)
{
	return m_camera;
}

} // namespace crown
