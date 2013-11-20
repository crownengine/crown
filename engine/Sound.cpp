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

#include "Sound.h"
#include "SoundResource.h"
#include "Vector3.h"
#include "Quaternion.h"
#include "Device.h"
#include "Unit.h"

namespace crown
{

//-----------------------------------------------------------------------------
void Sound::create(SoundResource* sr, int32_t node, const Vector3& /*pos*/, const Quaternion& /*rot*/)
{
	SoundRenderer* s = device()->sound_renderer();

	m_buffer = sr->m_id;
	m_source = s->create_sound_source();
	s->bind_buffer(m_buffer, m_source);

	m_node = node;
}

//-----------------------------------------------------------------------------
void Sound::destroy()
{
	device()->sound_renderer()->destroy_sound_source(m_source);
}

//-----------------------------------------------------------------------------
Vector3	Sound::local_position() const
{
	return m_local_pose.translation();
}

//-----------------------------------------------------------------------------
Quaternion Sound::local_rotation() const
{
	return m_local_pose.to_quaternion();
}

//-----------------------------------------------------------------------------
Matrix4x4 Sound::local_pose() const
{
	return m_local_pose;
}

//-----------------------------------------------------------------------------
Vector3	Sound::world_position() const
{
	return m_world_pose.translation();
}

//-----------------------------------------------------------------------------
Quaternion Sound::world_rotation() const
{
	return m_world_pose.to_quaternion();
}

//-----------------------------------------------------------------------------
Matrix4x4 Sound::world_pose() const
{
	return m_world_pose;
}

//-----------------------------------------------------------------------------
void Sound::set_local_position(Unit* unit, const Vector3& pos)
{
	m_local_pose.set_translation(pos);

	unit->set_local_position(pos, m_node);

}

//-----------------------------------------------------------------------------
void Sound::set_local_rotation(Unit* unit, const Quaternion& rot)
{
	Matrix4x4& local_pose = m_local_pose;

	Vector3 local_translation = local_pose.translation();
	local_pose = rot.to_matrix4x4();
	local_pose.set_translation(local_translation);

	unit->set_local_rotation(rot, m_node);
}

//-----------------------------------------------------------------------------
void Sound::set_local_pose(Unit* unit, const Matrix4x4& pose)
{
	m_local_pose = pose;

	unit->set_local_pose(pose, m_node);
}


} // namespace crown