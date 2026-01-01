/*
 * Copyright (c) 2012-2026 Daniele Bartolini et al.
 * SPDX-License-Identifier: MIT
 */

#include "config.h"
#include "core/error/error.inl"
#include "world/sound_world.h"

namespace crown
{
typedef void (*SoundWorldInitFunction)();
typedef void (*SoundWorldShutdownFunction)();
typedef SoundWorld *(*SoundWorldCreateFunction)(Allocator &, ResourceManager &);
typedef void (*SoundWorldDestroyFunction)(Allocator &, SoundWorld &);

#define SOUND_WORLD_IMPL(name)                                     \
	namespace sound_world_##name                                   \
	{                                                              \
		extern void init();                                        \
		extern void shutdown();                                    \
		extern SoundWorld *create(Allocator &, ResourceManager &); \
		extern void destroy(Allocator &, SoundWorld &);            \
	}

SOUND_WORLD_IMPL(al)
SOUND_WORLD_IMPL(noop)

#undef SOUND_WORLD_IMPL

namespace sound_world
{
	static SoundWorldInitFunction s_init_func;
	static SoundWorldShutdownFunction s_shutdown_func;
	static SoundWorldCreateFunction s_create_func;
	static SoundWorldDestroyFunction s_destroy_func;

	void init()
	{
		if (CROWN_SOUND_OPENAL) {
			s_init_func = sound_world_al::init;
			s_shutdown_func = sound_world_al::shutdown;
			s_create_func = sound_world_al::create;
			s_destroy_func = sound_world_al::destroy;
		} else if (CROWN_SOUND_NOOP) {
			s_init_func = sound_world_noop::init;
			s_shutdown_func = sound_world_noop::shutdown;
			s_create_func = sound_world_noop::create;
			s_destroy_func = sound_world_noop::destroy;
		} else {
			s_init_func = sound_world_noop::init;
			s_shutdown_func = sound_world_noop::shutdown;
			s_create_func = sound_world_noop::create;
			s_destroy_func = sound_world_noop::destroy;
		}

		CE_ENSURE(s_init_func != NULL);
		s_init_func();
	}

	void shutdown()
	{
		CE_ENSURE(s_shutdown_func != NULL);
		s_shutdown_func();
	}

	SoundWorld *create(Allocator &a, ResourceManager &rm)
	{
		SoundWorld *sw = NULL;

		CE_ENSURE(s_create_func != NULL);
		sw = s_create_func(a, rm);

		if (sw == NULL) {
			s_init_func = sound_world_noop::init;
			s_shutdown_func = sound_world_noop::shutdown;
			s_create_func = sound_world_noop::create;
			s_destroy_func = sound_world_noop::destroy;

			sw = s_create_func(a, rm);
			CE_ENSURE(sw != NULL);
		}

		return sw;
	}

	void destroy(Allocator &a, SoundWorld &sw)
	{
		CE_ENSURE(s_destroy_func != NULL);
		s_destroy_func(a, sw);
	}

} // namespace sound_world

} // namespace crown
