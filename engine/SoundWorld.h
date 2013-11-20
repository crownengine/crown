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

#include "IdArray.h"
#include "IdTable.h"
#include "Sound.h"
#include "List.h"
#include "Vector3.h"
#include "Quaternion.h"
#include "PoolAllocator.h"
#include "Resource.h"

#define MAX_SOUNDS 64

namespace crown
{

typedef Id SoundId;

class SoundWorld
{
public:
				SoundWorld();
				~SoundWorld();

	SoundId		create_sound(ResourceId id, int32_t node = -1, const Vector3& pos = Vector3::ZERO, const Quaternion& rot = Quaternion::IDENTITY);
	void		destroy_sound(SoundId id);
	Sound*		lookup_sound(SoundId id);

	void		update(float dt);

private:

	PoolAllocator					m_sounds_pool;
	IdArray<MAX_SOUNDS, Sound*>		m_sounds;
};

} // namespace crown