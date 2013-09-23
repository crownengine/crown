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

#include <SLES/OpenSLES.h>
#include <SLES/OpenSLES_Android.h>
#include <cstring>

#include "Types.h"
#include "Assert.h"
#include "SoundRenderer.h"
#include "SLESRenderer.h"
#include "Vec3.h"

namespace crown
{

//-----------------------------------------------------------------------------
class SoundRendererBackend
{
public:
	//-----------------------------------------------------------------------------
	void init()
	{
		SLresult result;

		const SLInterfaceID ids[] = {SL_IID_ENGINE};
		const SLboolean reqs[] = {SL_BOOLEAN_TRUE};

		result = slCreateEngine(&m_engine_obj, 0, NULL, 1, ids, reqs);
		result = (*m_engine_obj)->Realize(m_engine_obj, SL_BOOLEAN_FALSE);

		result = (*m_engine_obj)->GetInterface(m_engine_obj, SL_IID_ENGINE, &m_engine);

		const SLInterfaceID ids1[] = {SL_IID_VOLUME};
		const SLboolean reqs1[] = {SL_BOOLEAN_FALSE};

		result = (*m_engine)->CreateOutputMix(m_engine, &m_out_mix_obj, 1, ids1, reqs1); 
		result = (*m_out_mix_obj)->Realize(m_out_mix_obj, SL_BOOLEAN_FALSE);
	}

	//-----------------------------------------------------------------------------
	void shutdown()
	{
		for (uint32_t i = 0; i < MAX_SOUNDS; i++)
		{
			m_sounds[i].destroy();
		}

		if (m_out_mix_obj)
		{
			(*m_out_mix_obj)->Destroy(m_out_mix_obj);
			m_out_mix_obj = NULL;
		}

		if (m_engine_obj)
		{
			(*m_engine_obj)->Destroy(m_engine_obj);
			m_engine_obj = NULL;
			m_engine = NULL;
		}
	}

public:

	SLObjectItf						m_engine_obj;
	SLEngineItf						m_engine;

	SLObjectItf 					m_out_mix_obj;

	Sound 							m_sounds[MAX_SOUNDS];

private:

	friend class SoundRenderer;
};

//-----------------------------------------------------------------------------
SoundRenderer::SoundRenderer(Allocator& allocator) :
	m_allocator(allocator),
	m_backend(NULL),
	m_is_paused(false),
	m_num_sounds(0)
{
	m_backend = CE_NEW(m_allocator, SoundRendererBackend);
}

//-----------------------------------------------------------------------------
SoundRenderer::~SoundRenderer()
{
	if (m_backend)
	{
		CE_DELETE(m_allocator, m_backend);
	}
}

//-----------------------------------------------------------------------------
void SoundRenderer::init()
{
	m_backend->init();
}

//-----------------------------------------------------------------------------
void SoundRenderer::shutdown()
{
	m_backend->shutdown();
}

//-----------------------------------------------------------------------------
void SoundRenderer::pause()
{
	for (uint32_t i = 0; i < MAX_SOUNDS; i++)
	{
		if (m_backend->m_sounds[i].is_playing())
		{
			m_backend->m_sounds[i].pause();
		}
	}
}

//-----------------------------------------------------------------------------
void SoundRenderer::unpause()
{
	for (uint32_t i = 0; i < MAX_SOUNDS; i++)
	{
		if (m_backend->m_sounds[i].is_created() && !m_backend->m_sounds[i].is_playing())
		{
			m_backend->m_sounds[i].unpause();
		}
	}
}

//-----------------------------------------------------------------------------
void SoundRenderer::frame()
{
	// not needed right now
}

//-----------------------------------------------------------------------------
uint32_t SoundRenderer::num_sounds()
{
	return m_num_sounds;
}

//-----------------------------------------------------------------------------
SoundId SoundRenderer::create_sound(SoundResource* resource)
{
	SoundId id = m_sounds_id_table.create();

	m_backend->m_sounds[id.index].create(m_backend->m_engine, m_backend->m_out_mix_obj, resource);

	m_num_sounds++;

	return id;
}

//-----------------------------------------------------------------------------
void SoundRenderer::destroy_sound(SoundId id)
{
	CE_ASSERT(m_sounds_id_table.has(id), "Sound does not exists");

	m_backend->m_sounds[id.index].destroy();

	m_sounds_id_table.destroy(id);

	m_num_sounds--;
}

//-----------------------------------------------------------------------------
void SoundRenderer::play_sound(SoundId id)
{
	CE_ASSERT(m_sounds_id_table.has(id), "Sound does not exists");

	m_backend->m_sounds[id.index].play();
}

//-----------------------------------------------------------------------------
void SoundRenderer::pause_sound(SoundId id)
{
	CE_ASSERT(m_sounds_id_table.has(id), "Sound does not exists");

	m_backend->m_sounds[id.index].pause();
}

//-----------------------------------------------------------------------------
void SoundRenderer::set_sound_loop(SoundId id, bool loop)
{
	CE_ASSERT(m_sounds_id_table.has(id), "Sound does not exists");

	m_backend->m_sounds[id.index].loop(loop);
}

//-----------------------------------------------------------------------------
bool SoundRenderer::sound_playing(SoundId id)
{
	CE_ASSERT(m_sounds_id_table.has(id), "Sound does not exist");

	return m_backend->m_sounds[id.index].is_playing();
}

//-----------------------------------------------------------------------------
void SoundRenderer::set_listener(const Vec3& /*pos*/, const Vec3& /*vel*/, const Vec3& /*or_up*/, const Vec3& /*or_at*/) const
{
	Log::w("Stub");
}

//-----------------------------------------------------------------------------
void SoundRenderer::set_sound_min_distance(SoundId /*id*/,  const float /*min_distance*/)
{
	Log::w("Stub");
}

//-----------------------------------------------------------------------------
void SoundRenderer::set_sound_max_distance(SoundId /*id*/,  const float /*max_distance*/)
{
	Log::w("Stub");
}

//-----------------------------------------------------------------------------
void SoundRenderer::set_sound_position(SoundId /*id*/, const Vec3& /*pos*/)
{
	Log::w("Stub");
}

//-----------------------------------------------------------------------------
void SoundRenderer::set_sound_velocity(SoundId /*id*/, const Vec3& /*vel*/)
{
	Log::w("Stub");
}

//-----------------------------------------------------------------------------
void SoundRenderer::set_sound_direction(SoundId /*id*/, const Vec3& /*dir*/)
{
	Log::w("Stub");
}

//-----------------------------------------------------------------------------
void SoundRenderer::set_sound_pitch(SoundId /*id*/, const float /*pitch*/)
{
	Log::w("Stub");
}

//-----------------------------------------------------------------------------
void SoundRenderer::set_sound_gain(SoundId /*id*/, const float /*gain*/)
{
	Log::w("Stub");
}

//-----------------------------------------------------------------------------
void SoundRenderer::set_sound_rolloff(SoundId /*id*/, const float /*rolloff*/)
{
	Log::w("Stub");
}

//-----------------------------------------------------------------------------
float SoundRenderer::sound_min_distance(SoundId /*id*/) const
{
	Log::w("Stub");

	return 0;
}

//-----------------------------------------------------------------------------
float SoundRenderer::sound_max_distance(SoundId /*id*/) const
{
	Log::w("Stub");

	return 0;
}

//-----------------------------------------------------------------------------
Vec3 SoundRenderer::sound_position(SoundId /*id*/) const
{
	Log::w("Stub");

	return Vec3::ZERO;
}

//-----------------------------------------------------------------------------
Vec3 SoundRenderer::sound_velocity(SoundId /*id*/) const
{
	Log::w("Stub");

	return Vec3::ZERO;
}

//-----------------------------------------------------------------------------
Vec3 SoundRenderer::sound_direction(SoundId /*id*/) const
{
	Log::w("Stub");

	return Vec3::ZERO;
}

//-----------------------------------------------------------------------------
float SoundRenderer::sound_pitch(SoundId /*id*/) const
{
	Log::w("Stub");

	return 0;
}

//-----------------------------------------------------------------------------
float SoundRenderer::sound_gain(SoundId /*id*/) const
{
	Log::w("Stub");

	return 0;
}

//-----------------------------------------------------------------------------
float SoundRenderer::sound_rolloff(SoundId /*id*/) const
{
	Log::w("Stub");

	return 0;
}

//-----------------------------------------------------------------------------
int32_t SoundRenderer::sound_queued_buffers(SoundId id) const
{
	CE_ASSERT(m_sounds_id_table.has(id), "Sound does not exist");

	Log::w("Stub");
}

//-----------------------------------------------------------------------------
int32_t SoundRenderer::sound_processed_buffers(SoundId id) const
{
	CE_ASSERT(m_sounds_id_table.has(id), "Sound does not exist");

	Log::w("Stub");
}


} // namespace crown