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
#include "Unit.h"

namespace crown
{

//-----------------------------------------------------------------------------
void Sprite::create(SpriteResource* sr, int32_t node, const Vector3& /*pos*/, const Quaternion& /*rot*/)
{
	m_vb = sr->m_vb;
	m_ib = sr->m_ib;
	m_texture = sr->m_texture;
	m_node = node;

	m_animator = CE_NEW(default_allocator(), SpriteAnimator)(sr);
}

//-----------------------------------------------------------------------------
void Sprite::destroy()
{
	if (m_animator)
	{
		CE_DELETE(default_allocator(), m_animator);
	}
}

//-----------------------------------------------------------------------------
Vector3 Sprite::local_position() const
{
	Vector3 tmp = m_local_pose.translation();

	return tmp;
}

//-----------------------------------------------------------------------------
Quaternion Sprite::local_rotation() const
{
	Quaternion tmp = m_local_pose.to_quat();

	return tmp;
}

//-----------------------------------------------------------------------------
Matrix4x4 Sprite::local_pose() const
{
	return m_local_pose;
}

//-----------------------------------------------------------------------------
Vector3 Sprite::world_position() const
{
	Vector3 tmp = m_world_pose.translation();

	return tmp;
}

//-----------------------------------------------------------------------------
Quaternion Sprite::world_rotation() const
{
	Quaternion tmp = m_world_pose.to_quat();

	return tmp;
}

//-----------------------------------------------------------------------------
Matrix4x4 Sprite::world_pose() const
{
	return m_world_pose;	
}

//-----------------------------------------------------------------------------
void Sprite::set_local_position(Unit* unit, const Vector3& pos)
{
	m_local_pose.set_translation(pos);

	unit->set_local_position(pos, m_node);
}

//-----------------------------------------------------------------------------
void Sprite::set_local_rotation(Unit* unit, const Quaternion& rot)
{
	Matrix4x4& local_pose = m_local_pose;

	Vector3 local_translation = local_pose.translation();
	local_pose = rot.to_mat4();
	local_pose.set_translation(local_translation);

	unit->set_local_rotation(rot, m_node);
}

//-----------------------------------------------------------------------------
void Sprite::set_local_pose(Unit* unit, const Matrix4x4& pose)
{
	m_local_pose = pose;

	unit->set_local_pose(pose, m_node);
}


} // namespace crown