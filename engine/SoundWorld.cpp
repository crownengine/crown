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
#include "SoundRenderer.h"
#include "ResourceManager.h"

namespace crown
{

//-----------------------------------------------------------------------------
SoundWorld::SoundWorld()
	: m_sounds_pool(default_allocator(), MAX_SOUNDS, sizeof(Sound))
	, m_sounds(default_allocator())
{
	device()->sound_renderer()->set_listener(Vector3::ZERO, Vector3(1, 0, 0), Vector3(0, 1, 0), Vector3(1, 0, 0));
}

//-----------------------------------------------------------------------------
SoundWorld::~SoundWorld()
{
}

//-----------------------------------------------------------------------------
SoundId	SoundWorld::create_sound(ResourceId id, int32_t node, const Vector3& pos, const Quaternion& rot)
{
	SoundResource* sr = (SoundResource*) device()->resource_manager()->data(id);

	Sound* sound = (Sound*) m_sounds_pool.allocate(sizeof(Sound));

	const SoundId sound_id = m_sounds.create(sound);
	sound->create(sr, node, pos, rot);

	return sound_id;
}

//-----------------------------------------------------------------------------
void SoundWorld::destroy_sound(SoundId /*id*/)
{
	// Stub
}

//-----------------------------------------------------------------------------
Sound* SoundWorld::lookup_sound(SoundId id)
{
	CE_ASSERT(m_sounds.has(id), "Sound does not exits");

	return m_sounds.lookup(id);
}

//-----------------------------------------------------------------------------
void SoundWorld::update(float /*dt*/)
{
	List<Sound*>& sounds = m_sounds.m_objects; 
	for (uint32_t i = 0; i < sounds.size(); i++)
	{
		Sound* sound = sounds[i];

		device()->sound_renderer()->set_sound_loop(sound->m_source, sound->m_loop);
		device()->sound_renderer()->set_sound_gain(sound->m_source, sound->m_volume);
		device()->sound_renderer()->set_sound_max_distance(sound->m_source, sound->m_range);
		device()->sound_renderer()->set_sound_position(sound->m_source, sound->m_world_pose.translation());

		if (!sound->m_playing)
		{
			device()->sound_renderer()->play_sound(sound->m_source);
			sound->m_playing = true;
		}
	}
}


} // namespace crown