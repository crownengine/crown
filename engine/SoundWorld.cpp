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

#include "SoundWorld.h"
#include "Device.h"
#include "ResourceManager.h"
#include "SoundRenderer.h"
#include "SoundResource.h"
#include "Assert.h"

namespace crown
{

// //-----------------------------------------------------------------------------
// void SoundWorld::init()
// {

// }

// //-----------------------------------------------------------------------------
// void SoundWorld::shutdown()
// {

// }

//-----------------------------------------------------------------------------
SoundInstanceId SoundWorld::play_sound(const char* name, const bool loop, const Vec3& pos)
{
	SoundInstanceId id = m_sound_table.create();

	SoundResource* sound = (SoundResource*)device()->resource_manager()->lookup("sound", name);

	m_sound[id.index].m_sound = sound->m_id;

	device()->sound_renderer()->set_sound_loop(m_sound[id.index].m_sound, loop);

	device()->sound_renderer()->set_sound_position(m_sound[id.index].m_sound, pos);

	device()->sound_renderer()->play_sound(m_sound[id.index].m_sound);

	return id;
}

//-----------------------------------------------------------------------------
void SoundWorld::pause_sound(SoundInstanceId sound)
{
	CE_ASSERT(m_sound_table.has(sound), "SoundInstance does not exists");

	device()->sound_renderer()->pause_sound(sound);
}

// //-----------------------------------------------------------------------------
// void SoundWorld::link_sound(SoundInstanceId sound, UnitId unit)
// {
// 	CE_ASSERT(m_sound_table.has(sound), "SoundInstance does not exists");

// 	// Must be implemented
// }

//-----------------------------------------------------------------------------
void SoundWorld::set_listener(const Vec3& pos, const Vec3& vel, const Vec3& or_up, const Vec3& or_at)
{
	device()->sound_renderer()->set_listener(pos, vel, or_up, or_at);
}

//-----------------------------------------------------------------------------
void SoundWorld::set_sound_position(SoundInstanceId sound, const Vec3& pos)
{
	CE_ASSERT(m_sound_table.has(sound), "SoundInstance does not exists");

	device()->sound_renderer()->set_sound_position(sound, pos);
}

//-----------------------------------------------------------------------------
void SoundWorld::set_sound_range(SoundInstanceId sound, const float range)
{
	CE_ASSERT(m_sound_table.has(sound), "SoundInstance does not exists");

	device()->sound_renderer()->set_sound_max_distance(sound, range);
}

//-----------------------------------------------------------------------------
void SoundWorld::set_sound_volume(SoundInstanceId sound, const float vol)
{
	CE_ASSERT(m_sound_table.has(sound), "SoundInstance does not exists");

	device()->sound_renderer()->set_sound_gain(sound, vol);
}


} // namespace crown