--
-- Copyright (c) 2012-2014 Daniele Bartolini and individual contributors.
-- License: https://github.com/taylor001/crown/blob/master/LICENSE
--

project "openal"
	kind "SharedLib"

	configuration {}

	local AL_DIR = (CROWN_DIR .. "third/openal/")

	defines {
		"AL_ALEXT_PROTOTYPES",
		"AL_BUILD_LIBRARY",
		"_LARGEFILE_SOURCE",
		"_LARGE_FILES",
		"openal_EXPORTS",
	}

	configuration { "debug", "linux-*" }
		defines {
			"_DEBUG",
		}
		buildoptions {
			"-g3",
		}

	configuration { "development or release", "linux-*" }
		defines {
			"NDEBUG",
		}
		buildoptions {
			"-O2",
			"-fomit-frame-pointer",
		}

	configuration { "linux-*" }
		defines {
			-- These are needed on non-Windows systems for extra features
			"_GNU_SOURCE=1",
			"_POSIX_C_SOURCE=200809L",
			"_XOPEN_SOURCE=700",

			"HAVE_ALIGNED_ALLOC",
			"HAVE_POSIX_MEMALIGN",
			"HAVE_SSE",
			"HAVE_SSE2",
			"HAVE_ALSA",
			"HAVE_PULSEAUDIO",

			"HAVE_STAT",
			"HAVE_LRINTF",
			"HAVE_STRTOF",
			"SIZEOF_LONG=8",
			"SIZEOF_LONG_LONG=8",
			"HAVE_C99_VLA",
			"HAVE_C99_BOOL",
			"HAVE_C11_STATIC_ASSERT",
			"HAVE_C11_ALIGNAS",
			"HAVE_C11_ATOMIC",
			"HAVE_GCC_DESTRUCTOR",
			"HAVE_GCC_FORMAT",
			"HAVE_STDINT_H",
			"HAVE_STDBOOL_H",
			"HAVE_STDALIGN_H",
			"HAVE_DLFCN_H",
			"HAVE_MALLOC_H",
			"HAVE_FTW_H",
			"HAVE_STRINGS_H",
			"HAVE_CPUID_H",
			"HAVE_FLOAT_H",
			"HAVE_FENV_H",
			"HAVE_GCC_GET_CPUID",
			"HAVE_FTW",
			"HAVE_PTHREAD_SETSCHEDPARAM",
			"HAVE_PTHREAD_SETNAME_NP",
			"HAVE_PTHREAD_MUTEX_TIMEDLOCK",
		}
		buildoptions {
			"-D'AL_API=__attribute__((visibility(\"protected\")))'",
			"-D'ALC_API=__attribute__((visibility(\"protected\")))'",
			"-D'ALIGN(x)=__attribute__((aligned(x)))'",
			"-D'ALSOFT_VERSION=\"1.16.0\"'",
			"-std=c99",
			"-Winline",
			"-Wall",
			"-Wextra",
			"-fPIC",
			"-fvisibility=hidden",
			"-pthread",
		}
		files {
			AL_DIR .. "Alc/backends/alsa.c",
			AL_DIR .. "Alc/backends/pulseaudio.c",
		}

	includedirs {
		AL_DIR .. "include",
		AL_DIR .. "Alc",
		AL_DIR .. "OpenAL32/Include",
		AL_DIR .. "OpenAL32", -- for config.h
	}

	files {
		AL_DIR .. "common/atomic.c",
		AL_DIR .. "common/rwlock.c",
		AL_DIR .. "common/threads.c",
		AL_DIR .. "common/uintmap.c",

		AL_DIR .. "Alc/ALc.c",
		AL_DIR .. "Alc/ALu.c",
		AL_DIR .. "Alc/alcConfig.c",
		AL_DIR .. "Alc/alcRing.c",
		AL_DIR .. "Alc/bs2b.c",
		AL_DIR .. "Alc/effects/autowah.c",
		AL_DIR .. "Alc/effects/chorus.c",
		AL_DIR .. "Alc/effects/compressor.c",
		AL_DIR .. "Alc/effects/dedicated.c",
		AL_DIR .. "Alc/effects/distortion.c",
		AL_DIR .. "Alc/effects/echo.c",
		AL_DIR .. "Alc/effects/equalizer.c",
		AL_DIR .. "Alc/effects/flanger.c",
		AL_DIR .. "Alc/effects/modulator.c",
		AL_DIR .. "Alc/effects/null.c",
		AL_DIR .. "Alc/effects/reverb.c",
		AL_DIR .. "Alc/helpers.c",
		AL_DIR .. "Alc/hrtf.c",
		AL_DIR .. "Alc/panning.c",
		AL_DIR .. "Alc/mixer.c",
		AL_DIR .. "Alc/mixer_c.c",

		AL_DIR .. "Alc/midi/base.c",
		AL_DIR .. "Alc/midi/sf2load.c",
		AL_DIR .. "Alc/midi/dummy.c",
		AL_DIR .. "Alc/midi/fluidsynth.c",
		AL_DIR .. "Alc/midi/soft.c",

		-- Assume SSE2 available everywhere
		AL_DIR .. "Alc/mixer_sse2.c",
		AL_DIR .. "Alc/mixer_sse.c",

		AL_DIR .. "Alc/backends/base.c",
		AL_DIR .. "Alc/backends/loopback.c",
		AL_DIR .. "Alc/backends/null.c",

		AL_DIR .. "OpenAL32/alAuxEffectSlot.c",
		AL_DIR .. "OpenAL32/alBuffer.c",
		AL_DIR .. "OpenAL32/alEffect.c",
		AL_DIR .. "OpenAL32/alError.c",
		AL_DIR .. "OpenAL32/alExtension.c",
		AL_DIR .. "OpenAL32/alFilter.c",
		AL_DIR .. "OpenAL32/alFontsound.c",
		AL_DIR .. "OpenAL32/alListener.c",
		AL_DIR .. "OpenAL32/alMidi.c",
		AL_DIR .. "OpenAL32/alPreset.c",
		AL_DIR .. "OpenAL32/alSoundfont.c",
		AL_DIR .. "OpenAL32/alSource.c",
		AL_DIR .. "OpenAL32/alState.c",
		AL_DIR .. "OpenAL32/alThunk.c",
		AL_DIR .. "OpenAL32/sample_cvt.c",
	}

	configuration {}
