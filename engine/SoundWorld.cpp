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

//-----------------------------------------------------------------------------
SoundInstanceId SoundWorld::create_sound(const char* name)
{
	SoundInstanceId id = m_sound_table.create();

	SoundResource* sound = (SoundResource*)device()->resource_manager()->lookup("sound", name);

	m_sound[id.index].m_sound = sound->m_id;

	return id;
}

//-----------------------------------------------------------------------------
void SoundWorld::destroy_sound(SoundInstanceId sound)
{
	CE_ASSERT(m_sound_table.has(sound), "SoundInstance does not exists");

	// Stub
}

} // namespace crown