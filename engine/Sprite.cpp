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

#include "Sprite.h"
#include "Vector3.h"
#include "Quaternion.h"
#include "SpriteResource.h"
#include "Allocator.h"
#include "SpriteAnimator.h"
#include "SceneGraph.h"
#include "Unit.h"

namespace crown
{

//-----------------------------------------------------------------------------
Sprite::Sprite(SceneGraph& sg, int32_t node, const SpriteResource* sr)
	: m_scene_graph(sg)
	, m_node(node)
	, m_resource(sr)
{
	m_vb = sr->vertex_buffer();
	m_ib = sr->index_buffer();
	m_texture = ((TextureResource*)device()->resource_manager()->data(sr->texture()))->texture();
	m_animator = CE_NEW(default_allocator(), SpriteAnimator)(sr);
}

//-----------------------------------------------------------------------------
Sprite::~Sprite()
{
	CE_DELETE(default_allocator(), m_animator);
}

//-----------------------------------------------------------------------------
Vector3 Sprite::local_position() const
{
	return m_scene_graph.local_position(m_node);
}

//-----------------------------------------------------------------------------
Quaternion Sprite::local_rotation() const
{
	return m_scene_graph.local_rotation(m_node);
}

//-----------------------------------------------------------------------------
Matrix4x4 Sprite::local_pose() const
{
	return m_scene_graph.local_pose(m_node);
}

//-----------------------------------------------------------------------------
Vector3 Sprite::world_position() const
{
	return m_scene_graph.world_position(m_node);
}

//-----------------------------------------------------------------------------
Quaternion Sprite::world_rotation() const
{
	return m_scene_graph.world_rotation(m_node);
}

//-----------------------------------------------------------------------------
Matrix4x4 Sprite::world_pose() const
{
	return m_scene_graph.world_pose(m_node);
}

//-----------------------------------------------------------------------------
void Sprite::set_local_position(Unit* unit, const Vector3& pos)
{
	unit->set_local_position(m_node, pos);
}

//-----------------------------------------------------------------------------
void Sprite::set_local_rotation(Unit* unit, const Quaternion& rot)
{
	unit->set_local_rotation(m_node, rot);
}

//-----------------------------------------------------------------------------
void Sprite::set_local_pose(Unit* unit, const Matrix4x4& pose)
{
	unit->set_local_pose(m_node, pose);
}

} // namespace crown
