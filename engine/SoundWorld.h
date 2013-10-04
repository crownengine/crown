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

#include "IdTable.h"
#include "SoundRenderer.h"
#include "Vec3.h"
#include "Mat4.h"

namespace crown
{

typedef Id SoundInstanceId;

struct SoundInstance
{
	SoundId m_sound;
};

class SoundWorld
{
public:
	// void						init();
	// void						shutdown();

	SoundInstanceId				play_sound(const char* name, const bool loop = false, const Vec3& pos = Vec3::ZERO);
	void						pause_sound(SoundInstanceId sound);

	// void 						link_sound(SoundInstanceId sound, UnitId unit);

	void						set_listener(const Vec3& pos, const Vec3& vel, const Vec3& or_up, const Vec3& or_at);

	void						set_sound_position(SoundInstanceId sound, const Vec3& pos);
	void						set_sound_range(SoundInstanceId sound, const float range);
	void						set_sound_volume(SoundInstanceId sound, const float vol);

private:

	IdTable<MAX_SOUNDS> 		m_sound_table;
	SoundInstance				m_sound[MAX_SOUNDS];
};

} // namespace crown