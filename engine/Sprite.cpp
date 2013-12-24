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
#include "SceneGraph.h"
#include "Unit.h"
#include "Renderer.h"
#include "Material.h"
#include "RenderWorld.h"

namespace crown
{

//-----------------------------------------------------------------------------
Sprite::Sprite(RenderWorld& render_world, SceneGraph& sg, int32_t node, const SpriteResource* sr)
	: m_render_world(render_world)
	, m_scene_graph(sg)
	, m_node(node)
	, m_resource(sr)
{
	m_vb = sr->vertex_buffer();
	m_ib = sr->index_buffer();
}

//-----------------------------------------------------------------------------
Sprite::~Sprite()
{
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

//-----------------------------------------------------------------------------
void Sprite::set_material(MaterialId mat)
{
	m_material = mat;
}

//-----------------------------------------------------------------------------
void Sprite::render(Renderer& r, UniformId uniform)
{
	Material* material = m_render_world.lookup_material(m_material);
	material->bind(r, uniform);

	r.set_state(STATE_DEPTH_WRITE 
		| STATE_COLOR_WRITE 
		| STATE_ALPHA_WRITE 
		| STATE_CULL_CW 
		| STATE_BLEND_EQUATION_ADD 
		| STATE_BLEND_FUNC(STATE_BLEND_FUNC_SRC_ALPHA, STATE_BLEND_FUNC_ONE_MINUS_SRC_ALPHA));
	r.set_vertex_buffer(m_vb);
	r.set_index_buffer(m_ib);
	r.set_pose(world_pose());
	r.commit(0);
}

} // namespace crown
