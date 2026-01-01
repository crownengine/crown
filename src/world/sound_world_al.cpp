/*
 * Copyright (c) 2012-2026 Daniele Bartolini et al.
 * SPDX-License-Identifier: MIT
 */

#include "config.h"

#if CROWN_SOUND_OPENAL
#include "core/containers/array.inl"
#include "core/filesystem/file.h"
#include "core/math/constants.h"
#include "core/math/matrix4x4.inl"
#include "core/strings/string_id.inl"
#include "core/math/vector3.inl"
#include "core/memory/temp_allocator.inl"
#include "device/log.h"
#include "resource/resource_manager.h"
#include "resource/sound_resource.h"
#include "resource/sound_ogg.h"
#include "world/sound_world.h"
#include <AL/al.h>
#include <AL/alc.h>
#include <AL/alext.h>
#define STB_VORBIS_HEADER_ONLY
#define STB_VORBIS_NO_PULLDATA_API
#include <stb_vorbis.c>

LOG_SYSTEM(SOUND, "sound")

namespace crown
{
#if CROWN_DEBUG
static const char *al_error_to_string(ALenum error)
{
	switch (error) {
	case AL_INVALID_ENUM: return "AL_INVALID_ENUM";
	case AL_INVALID_VALUE: return "AL_INVALID_VALUE";
	case AL_INVALID_OPERATION: return "AL_INVALID_OPERATION";
	case AL_OUT_OF_MEMORY: return "AL_OUT_OF_MEMORY";
	default: return "UNKNOWN_AL_ERROR";
	}
}

	#define AL_CHECK(function)                              \
		function;                                           \
		do                                                  \
		{                                                   \
			ALenum error;                                   \
			CE_ASSERT((error = alGetError()) == AL_NO_ERROR \
				, "alGetError: %s"                          \
				, al_error_to_string(error)                 \
				);                                          \
		} while (0)
#else
	#define AL_CHECK(function) function
#endif // if CROWN_DEBUG

struct SoundInstance
{
	SoundInstanceId _id;

	const SoundResource *_resource;
	StringId64 _name;
	bool _loop;
	f32 _volume;
	StringId32 _group;
	f32 _range;
	u32 _flags;

	ALuint _buffer[SOUND_MAX_BUFFERS];
	u32 _num_buffers;
	ALenum _format;
	ALuint _source;

	u32 _block_samples; ///< Number of samples in each block per channel.
	u32 _block_size;    ///< Size of each block of samples in bytes.

	void *_stream_data;  ///< Total memory to allow streaming.
	u8 *_stream_decoded; ///< Current block of decoded audio samples.
	u8 *_stream_encoded; ///< Current block of encoded streaming data.
	u32 _stream_pos;     ///< Size of encoded data.
	File *_stream;       ///< Streaming data source.

	// Vorbis-specific.
	stb_vorbis_alloc *_vorbis_alloc;
	unsigned char *_vorbis_headers;
	stb_vorbis *_vorbis;

	void create(const SoundResource *sr, File *stream, bool loop, f32 range, u32 flags, const Vector3 &pos, StringId32 group)
	{
		_resource = sr;
		_loop = loop;
		_stream_data = NULL;
		_stream = stream;
		_vorbis = NULL;
		_volume = 1.0f;
		_group = group;
		_flags = flags;

		// Create source.
		AL_CHECK(alGenSources(1, &_source));
		CE_ASSERT(alIsSource(_source), "alGenSources: error");

		bool is_2d = !(flags & PlaySoundFlags::ENABLE_ATTENUATION);

		AL_CHECK(alSourcef(_source, AL_REFERENCE_DISTANCE, 1.0f));
		AL_CHECK(alSourcef(_source, AL_ROLLOFF_FACTOR, is_2d ? 0.0f : 1.0f));
		AL_CHECK(alSourcei(_source, AL_SOURCE_RELATIVE, is_2d ? AL_TRUE : AL_FALSE));
		AL_CHECK(alSourcef(_source, AL_PITCH, 1.0f));
		set_position(pos);
		_range = range;

		switch (sr->bit_depth) {
		case  8: _format = sr->channels > 1 ? AL_FORMAT_STEREO8  : AL_FORMAT_MONO8; break;
		case 16: _format = sr->channels > 1 ? AL_FORMAT_STEREO16 : AL_FORMAT_MONO16; break;
		case 32: _format = sr->channels > 1 ? AL_FORMAT_STEREO_FLOAT32 : AL_FORMAT_MONO_FLOAT32; break;
		default: CE_FATAL("Number of bits per sample not supported."); break;
		}

		// Generate buffers.
		_block_samples = sr->sample_rate * SOUND_BUFFER_MS / 1000;
		_block_size = (sr->bit_depth / 8) * _block_samples * sr->channels;
		_num_buffers = SOUND_MAX_BUFFERS;
		if (sr->stream_format == StreamFormat::NONE) {
			// If there's no stream, use a single buffer and
			// make AL handle the looping machinery for us.
			_num_buffers = 1;
			AL_CHECK(alSourcei(_source, AL_LOOPING, (loop ? AL_TRUE : AL_FALSE)));
		}
		AL_CHECK(alGenBuffers(_num_buffers, &_buffer[0]));

		fill_buffers();
	}

	// Fill buffers with pre-decoded samples.
	void fill_buffers()
	{
		const u8 *pcm_data = sound_resource::pcm_data(_resource);
		u32 bs = _block_size;

		if (_num_buffers == 1)
			bs = _resource->pcm_size;

		for (u32 i = 0, p = 0; i < _num_buffers && p != _resource->pcm_size; ++i) {
			const u32 num = min(bs, _resource->pcm_size - p);
			AL_CHECK(alBufferData(_buffer[i], _format, &pcm_data[p], num, _resource->sample_rate));
			AL_CHECK(alSourceQueueBuffers(_source, 1, &_buffer[i]));
			p += num;
		}
	}

	void create(ResourceManager *rm, StringId64 name, bool loop, f32 range, u32 flags, const Vector3 &pos, StringId32 group)
	{
		const SoundResource *sr = (SoundResource *)rm->get(RESOURCE_TYPE_SOUND, name);

		File *stream = NULL;
		if (sr->stream_format != StreamFormat::NONE)
			stream = rm->open_stream(RESOURCE_TYPE_SOUND, name);

		_name = name;
		return create(sr, stream, loop, range, flags, pos, group);
	}

	/// Decodes a block of samples of size BLOCK_MS or less, and feeds it to AL.
	/// Returns the number of samples that have been decoded.
	u32 decode_samples()
	{
		if (!_stream)
			return 0;

		if (_resource->stream_format == StreamFormat::OGG) {
			const OggStreamMetadata *ogg = (OggStreamMetadata *)sound_resource::stream_metadata(_resource);
			int used;
			int skip_n = 0;

			// Open the stream.
			if (_stream_data == NULL) {
				const u32 stream_mem_size = 0
					+ sizeof(stb_vorbis_alloc) + alignof(stb_vorbis_alloc)
					+ ogg->alloc_buffer_size
					+ ogg->headers_size
					+ ogg->max_frame_size
					+ _block_size*2 + alignof(f32)
					;
				_stream_data = default_allocator().allocate(stream_mem_size);

				_vorbis_alloc = (stb_vorbis_alloc *)memory::align_top(_stream_data, alignof(stb_vorbis_alloc));
				_vorbis_alloc->alloc_buffer = (char *)&_vorbis_alloc[1];
				_vorbis_alloc->alloc_buffer_length_in_bytes = ogg->alloc_buffer_size;

				_vorbis_headers = (unsigned char *)&_vorbis_alloc[1] + ogg->alloc_buffer_size;

				_stream_encoded = (unsigned char *)_vorbis_headers + ogg->headers_size;
				_stream_decoded = (u8 *)memory::align_top(_stream_encoded + ogg->max_frame_size, alignof(f32));
				_stream_pos = 0;

				_stream->read(_vorbis_headers, ogg->headers_size);
				skip_n = ogg->num_samples_skip;
			}

			if (_vorbis == NULL) {
				int error;
				_vorbis = stb_vorbis_open_pushdata(_vorbis_headers, ogg->headers_size, &used, &error, _vorbis_alloc);
				CE_ENSURE(error == VORBIS__no_error);
				CE_ENSURE(_vorbis != NULL);
				CE_ENSURE(used == ogg->headers_size);
			}

			// Decode _block_size samples or less.
			f32 *dec = (f32 *)_stream_decoded;
			u32 tot_n = 0;
			while (tot_n < _block_samples) {
				float **output;
				int n;
				int q = ogg->max_frame_size;

			retry:
				if (q > int(_stream->size() - _stream_pos))
					q = _stream->size() - _stream_pos;
				if ((int)_stream_pos < q)
					_stream_pos += _stream->read(&_stream_encoded[_stream_pos], q - _stream_pos);

				used = stb_vorbis_decode_frame_pushdata(_vorbis
					, _stream_encoded
					, _stream_pos
					, NULL
					, &output
					, &n
					);

				if (used == 0) {
					if (_stream->end_of_file()) {
						if (_loop) {
							stb_vorbis_close(_vorbis);
							_vorbis = NULL;
							_stream->seek(ogg->headers_size);
							_stream_pos = 0;
						}
						break;
					}

					goto retry; // Need more data.
				}

				_stream_pos = q - used;
				memmove(_stream_encoded, &_stream_encoded[used], _stream_pos);
				if (n == 0)
					continue; // Seek/error recovery.

				// Skip unwanted samples.
				if (skip_n >= n) {
					skip_n -= n;
					continue;
				} else {
					n -= skip_n;
				}

				for (int i = 0; i < n; ++i) {
					*dec++ = output[0][i + skip_n];
					*dec++ = output[1][i + skip_n];
				}
				skip_n = 0;
				tot_n += n;
			}

			return tot_n;
		} else {
			CE_FATAL("Unknown stream format");
			return 0;
		}
	}

	void update()
	{
		ALint processed;
		AL_CHECK(alGetSourcei(_source, AL_BUFFERS_PROCESSED, &processed));

		while (processed > 0) {
			ALuint buffer;
			AL_CHECK(alSourceUnqueueBuffers(_source, 1, &buffer));
			--processed;

			// Decode a block of samples and enqueue it.
			u32 n = decode_samples();
			if (n > 0) {
				const u32 size = n * _resource->channels * _resource->bit_depth / 8;
				AL_CHECK(alBufferData(buffer, _format, _stream_decoded, size, _resource->sample_rate));
				AL_CHECK(alSourceQueueBuffers(_source, 1, &buffer));
			}
		}

		ALint state;
		AL_CHECK(alGetSourcei(_source, AL_SOURCE_STATE, &state));

		if (state != AL_PLAYING && state != AL_PAUSED) {
			// Either the source underrun or no buffers were enqueued.
			ALint queued;
			AL_CHECK(alGetSourcei(_source, AL_BUFFERS_QUEUED, &queued));
			if (queued == 0)
				return; // Finished.

			// Underrun, restart playback.
			AL_CHECK(alSourcePlay(_source));
		}
	}

	void destroy(ResourceManager *rm)
	{
		stop();
		AL_CHECK(alSourcei(_source, AL_BUFFER, 0));
		AL_CHECK(alDeleteBuffers(_num_buffers, &_buffer[0]));
		AL_CHECK(alDeleteSources(1, &_source));

		// Deallocate streaming memory.
		stb_vorbis_close(_vorbis);
		default_allocator().deallocate(_stream_data);

		if (_stream != NULL)
			rm->close_stream(_stream);
	}

	void reload(ResourceManager *rm)
	{
		destroy(rm);
		create(rm, _name, _loop, _range, _flags, position(), _group);
	}

	void pause()
	{
		AL_CHECK(alSourcePause(_source));
	}

	void resume()
	{
		AL_CHECK(alSourcePlay(_source));
	}

	void stop()
	{
		AL_CHECK(alSourceStop(_source));
		AL_CHECK(alSourceRewind(_source)); // Workaround
		ALint processed;
		AL_CHECK(alGetSourcei(_source, AL_BUFFERS_PROCESSED, &processed));

		if (processed > 0) {
			ALuint removed;
			AL_CHECK(alSourceUnqueueBuffers(_source, 1, &removed));
		}
	}

	bool is_playing()
	{
		ALint state;
		AL_CHECK(alGetSourcei(_source, AL_SOURCE_STATE, &state));
		return state == AL_PLAYING;
	}

	bool finished()
	{
		ALint state;
		AL_CHECK(alGetSourcei(_source, AL_SOURCE_STATE, &state));
		return state != AL_PLAYING && state != AL_PAUSED;
	}

	Vector3 position()
	{
		ALfloat pos[3];
		AL_CHECK(alGetSourcefv(_source, AL_POSITION, pos));
		return { pos[0], pos[1], pos[2] };
	}

	void set_position(const Vector3 &pos)
	{
		AL_CHECK(alSourcefv(_source, AL_POSITION, to_float_ptr(pos)));
	}
};

#define MAX_OBJECTS       1024
#define INDEX_MASK        0xffff
#define NEW_OBJECT_ID_ADD 0x10000

struct SoundWorldAL : public SoundWorld
{
	struct Index
	{
		SoundInstanceId id;
		u16 index;
		u16 next;
	};

	struct SoundGroup
	{
		StringId32 name;
		f32 volume;
	};

	u32 _marker;
	Allocator *_allocator;
	ResourceManager *_resource_manager;
	u32 _num_objects;
	SoundInstance _playing_sounds[MAX_OBJECTS];
	Index _indices[MAX_OBJECTS];
	u16 _freelist_enqueue;
	u16 _freelist_dequeue;
	Matrix4x4 _listener_pose;
	Array<SoundGroup> _groups;

	SoundWorldAL(Allocator &a, ResourceManager &rm)
		: _marker(SOUND_WORLD_MARKER)
		, _allocator(&a)
		, _resource_manager(&rm)
		, _groups(a)
	{
		_num_objects = 0;
		for (u32 i = 0; i < MAX_OBJECTS; ++i) {
			_indices[i].id = i;
			_indices[i].next = i + 1;
		}
		_freelist_dequeue = 0;
		_freelist_enqueue = MAX_OBJECTS - 1;

		set_listener_pose(MATRIX4X4_IDENTITY);
	}

	SoundWorldAL(const SoundWorldAL &) = delete;
	SoundWorldAL &operator=(const SoundWorldAL &) = delete;

	virtual ~SoundWorldAL()
	{
		for (u32 i = 0; i < _num_objects; ++i) {
			SoundInstance &inst = lookup(_playing_sounds[i]._id);
			inst.destroy(_resource_manager);
		}

		_marker = 0;
	}

	bool has(SoundInstanceId id)
	{
		const Index &in = _indices[id & INDEX_MASK];
		return in.id == id && in.index != UINT16_MAX;
	}

	SoundInstance &lookup(SoundInstanceId id)
	{
		return _playing_sounds[_indices[id & INDEX_MASK].index];
	}

	SoundInstanceId add()
	{
		Index &in = _indices[_freelist_dequeue];
		_freelist_dequeue = in.next;
		in.id += NEW_OBJECT_ID_ADD;
		in.index = _num_objects++;
		SoundInstance &o = _playing_sounds[in.index];
		o._id = in.id;
		return o._id;
	}

	void remove(SoundInstanceId id)
	{
		Index &in = _indices[id & INDEX_MASK];

		SoundInstance &o = _playing_sounds[in.index];
		o = _playing_sounds[--_num_objects];
		_indices[o._id & INDEX_MASK].index = in.index;

		in.index = UINT16_MAX;
		_indices[_freelist_enqueue].next = id & INDEX_MASK;
		_freelist_enqueue = id & INDEX_MASK;
	}

	SoundInstanceId play(StringId64 name
		, bool loop
		, f32 volume
		, f32 range
		, u32 flags
		, const Vector3 &pos
		, StringId32 group
		)
	{
		// Create sound group if it does not exist.
		u32 i, n;
		for (i = 0, n = array::size(_groups); i < n; ++i) {
			if (_groups[i].name == group && group != StringId32(0u))
				break;
		}
		if (i == n)
			array::push_back(_groups, { group, 1.0f });

		// Spawn sound instance.
		SoundInstanceId id = add();
		SoundInstance &inst = lookup(id);
		inst.create(_resource_manager, name, loop, range, flags, pos, group);
		set_sound_volumes(1, &id, &volume);

		if (distance_squared(translation(_listener_pose), inst.position()) > inst._range*inst._range) {
			AL_CHECK(alSourcef(inst._source, AL_GAIN, 0.0f));
		}

		AL_CHECK(alSourcePlay(inst._source));
		return id;
	}

	void stop(SoundInstanceId id)
	{
		SoundInstance &si = lookup(id);
		si.destroy(_resource_manager);
		remove(id);
	}

	bool is_playing(SoundInstanceId id)
	{
		return has(id) && lookup(id).is_playing();
	}

	void stop_all()
	{
		for (u32 i = 0; i < _num_objects; ++i) {
			_playing_sounds[i].stop();
		}
	}

	void pause_all()
	{
		for (u32 i = 0; i < _num_objects; ++i) {
			_playing_sounds[i].pause();
		}
	}

	void resume_all()
	{
		for (u32 i = 0; i < _num_objects; ++i) {
			_playing_sounds[i].resume();
		}
	}

	void set_sound_positions(u32 num, const SoundInstanceId *ids, const Vector3 *positions)
	{
		for (u32 i = 0; i < num; ++i) {
			lookup(ids[i]).set_position(positions[i]);
		}
	}

	void set_sound_ranges(u32 num, const SoundInstanceId *ids, const f32 *ranges)
	{
		for (u32 i = 0; i < num; ++i) {
			lookup(ids[i])._range = ranges[i];
		}
	}

	void set_sound_volumes(u32 num, const SoundInstanceId *ids, const f32 *volumes)
	{
		for (u32 i = 0; i < num; i++) {
			SoundInstance &inst = lookup(ids[i]);
			f32 volume = clamp(volumes[i], 0.0f, 1.0f);
			f32 group_volume = 1.0f;

			// Find group's volume.
			for (u32 i = 0; i < array::size(_groups); ++i) {
				if (_groups[i].name == inst._group) {
					group_volume = _groups[i].volume;
					break;
				}
			}

			AL_CHECK(alSourcef(inst._source, AL_GAIN, volume * group_volume));
			inst._volume = volume;
		}
	}

	void reload_sounds(const SoundResource *old_sr, const SoundResource *new_sr)
	{
		CE_UNUSED(new_sr);

		for (u32 i = 0; i < _num_objects; ++i) {
			if (_playing_sounds[i]._resource == old_sr) {
				_playing_sounds[i].reload(_resource_manager);
			}
		}
	}

	void set_listener_pose(const Matrix4x4 &pose)
	{
		const Vector3 pos = translation(pose);
		const Vector3 up = z(pose);
		const Vector3 at = y(pose);

		AL_CHECK(alListener3f(AL_POSITION, pos.x, pos.y, pos.z));
		// AL_CHECK(alListener3f(AL_VELOCITY, vel.x, vel.y, vel.z));

		const ALfloat orientation[] = { at.x, at.y, at.z, up.x, up.y, up.z };
		AL_CHECK(alListenerfv(AL_ORIENTATION, orientation));
		_listener_pose = pose;
	}

	void set_group_volume(StringId32 group, f32 volume)
	{
		u32 i, n;
		for (i = 0, n = array::size(_groups); i < n; ++i) {
			if (_groups[i].name == group) {
				_groups[i].volume = volume;
				break;
			}
		}
		if (i == n)
			array::push_back(_groups, { group, volume });

		for (u32 i = 0; i < _num_objects; ++i) {
			SoundInstance &inst = _playing_sounds[i];
			set_sound_volumes(1, &inst._id, &inst._volume);
		}
	}

	void update()
	{
		TempAllocator256 alloc;
		Array<SoundInstanceId> to_delete(alloc);

		Vector3 listener_pos = translation(_listener_pose);

		// Update instances with new samples.
		for (u32 i = 0; i < _num_objects; ++i) {
			SoundInstance &inst = _playing_sounds[i];

			inst.update();
			if (inst.finished()) {
				array::push_back(to_delete, inst._id);
			} else {
				if (distance_squared(listener_pos, inst.position()) > inst._range*inst._range) {
					AL_CHECK(alSourcef(inst._source, AL_GAIN, 0.0f));
				} else {
					set_sound_volumes(1, &inst._id, &inst._volume);
				}
			}
		}

		// Destroy instances which finished playing.
		for (u32 i = 0; i < array::size(to_delete); ++i) {
			stop(to_delete[i]);
		}
	}
};

namespace sound_world_al
{
	static ALCdevice *s_al_device;
	static ALCcontext *s_al_context;

	void init()
	{
		s_al_device = alcOpenDevice(NULL);

		if (s_al_device == NULL) {
			logw(SOUND, "Failed to open audio device");
			return;
		}

		s_al_context = alcCreateContext(s_al_device, NULL);
		CE_ASSERT(s_al_context, "alcCreateContext: error");

		AL_CHECK(alcMakeContextCurrent(s_al_context));

#if CROWN_DEBUG && !CROWN_DEVELOPMENT
		logi(SOUND, "OpenAL Vendor   : %s", alGetString(AL_VENDOR));
		logi(SOUND, "OpenAL Version  : %s", alGetString(AL_VERSION));
		logi(SOUND, "OpenAL Renderer : %s", alGetString(AL_RENDERER));
#endif

		AL_CHECK(alDistanceModel(AL_INVERSE_DISTANCE_CLAMPED));
		AL_CHECK(alDopplerFactor(1.0f));
		AL_CHECK(alDopplerVelocity(343.0f));
	}

	void shutdown()
	{
		alcDestroyContext(s_al_context);
		alcCloseDevice(s_al_device);
	}

	SoundWorld *create(Allocator &a, ResourceManager &rm)
	{
		return s_al_device != NULL
			? CE_NEW(a, SoundWorldAL)(a, rm)
			: NULL
			;
	}

	void destroy(Allocator &a, SoundWorld &sw)
	{
		CE_DELETE(a, &sw);
	}

} // namespace sound_world_al

} // namespace crown

#endif // if CROWN_SOUND_OPENAL
