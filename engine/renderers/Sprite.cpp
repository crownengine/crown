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
	, m_frame(0)
	, m_animation(NULL)
	, m_time(0)
	, m_loop(false)
{
	Blob vmem = sr->get_vertices();
	Blob imem = sr->get_indices();

	bgfx::VertexDecl decl;
	decl.begin()
		.add(bgfx::Attrib::Position, 2, bgfx::AttribType::Float)
		.add(bgfx::Attrib::TexCoord0, 2, bgfx::AttribType::Float, true)
		.end();

	m_vb = bgfx::createVertexBuffer(bgfx::makeRef((void*) vmem.m_data, vmem.m_size), decl);
	m_ib = bgfx::createIndexBuffer(bgfx::makeRef((void*) imem.m_data, imem.m_size));
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
void Sprite::set_frame(uint32_t i)
{
	m_frame = i;
}

//-----------------------------------------------------------------------------
void Sprite::play_animation(const char* name, bool loop)
{
	if (m_animation)
		return;

	m_animation = m_resource->get_animation(name);
	m_time = 0;
	m_loop = loop;
	m_frame = m_resource->get_animation_frame(m_animation, m_animation->start_frame);
}

//-----------------------------------------------------------------------------
void Sprite::stop_animation()
{
	m_animation = NULL;
	m_time = 0;
	m_frame = 0;
}

//-----------------------------------------------------------------------------
void Sprite::update(float dt)
{
	if (!m_animation)
		return;

	m_time += dt;

	if (m_time >= m_animation->time)
	{
		if (m_loop)
		{
			m_time = 0;
		}
		else
		{
			stop_animation();
			return;
		}
	}

	uint32_t frame = m_animation->num_frames * (m_time / m_animation->time);
	m_frame = m_resource->get_animation_frame(m_animation, frame);
}

//-----------------------------------------------------------------------------
void Sprite::render()
{
	// Material* material = m_render_world.get_material(m_material);
	// material->bind(r, uniform);

	///
	/// @param _state State flags. Default state for primitive type is
	///   triangles. See: BGFX_STATE_DEFAULT.
	///
	///   BGFX_STATE_ALPHA_WRITE - Enable alpha write.
	///   BGFX_STATE_DEPTH_WRITE - Enable depth write.
	///   BGFX_STATE_DEPTH_TEST_* - Depth test function.
	///   BGFX_STATE_BLEND_* - See NOTE 1: BGFX_STATE_BLEND_FUNC.
	///   BGFX_STATE_BLEND_EQUATION_* - See NOTE 2.
	///   BGFX_STATE_CULL_* - Backface culling mode.
	///   BGFX_STATE_RGB_WRITE - Enable RGB write.
	///   BGFX_STATE_MSAA - Enable MSAA.
	///   BGFX_STATE_PT_[LINES/POINTS] - Primitive type.
	///
	/// @param _rgba Sets blend factor used by BGFX_STATE_BLEND_FACTOR and
	///   BGFX_STATE_BLEND_INV_FACTOR blend modes.
	///
	/// NOTE:
	///   1. Use BGFX_STATE_ALPHA_REF, BGFX_STATE_POINT_SIZE and
	///      BGFX_STATE_BLEND_FUNC macros to setup more complex states.
	///   2. BGFX_STATE_BLEND_EQUATION_ADD is set when no other blend
	///      equation is specified.
	///
	bgfx::setState(BGFX_STATE_DEFAULT);

	// r.set_state(STATE_DEPTH_WRITE 
	// 	| STATE_COLOR_WRITE 
	// 	| STATE_ALPHA_WRITE 
	// 	| STATE_CULL_CW 
	// 	| STATE_BLEND_EQUATION_ADD 
	// 	| STATE_BLEND_FUNC(STATE_BLEND_FUNC_SRC_ALPHA, STATE_BLEND_FUNC_ONE_MINUS_SRC_ALPHA));

	bgfx::setVertexBuffer(m_vb);
	bgfx::setIndexBuffer(m_ib, m_frame * 6, 6);
	bgfx::setTransform(matrix4x4::to_float_ptr(world_pose()));
	bgfx::submit(0);
}

} // namespace crown
