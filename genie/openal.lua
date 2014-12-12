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
	}

	configuration { "debug" }
		defines {
			"_DEBUG",
		}

	configuration { "development or release" }
		defines {
			"NDEBUG",
		}

	configuration { "development or release", "linux-*" }
		buildoptions {
			"-O2",
			"-fomit-frame-pointer",
		}

	configuration { "vs*" }
		defines {
			"WIN32",
			"_WINDOWS",
			"_WIN32",
			"_WIN32_WINNT=0x0502",
			"restrict=",
			"inline=__inline",
			"_CRT_SECURE_NO_WARNINGS",
			"_CRT_NONSTDC_NO_DEPRECATE",
			"strcasecmp=_stricmp",
			"strncasecmp=_strnicmp",
			"snprintf=_snprintf",
		}
		buildoptions {
			"/wd4098",
		}
		includedirs {
			AL_DIR .. "OpenAL32/config_vs2013",
		}
		files {
			AL_DIR .. "Alc/backends/mmdevapi.c",
			AL_DIR .. "Alc/backends/dsound.c",
			AL_DIR .. "Alc/backends/winmm.c",
		}
		links {
			"winmm",
		}

	configuration { "linux-*" }
		defines {
			-- These are needed on non-Windows systems for extra features
			"_GNU_SOURCE=1",
			"_POSIX_C_SOURCE=200809L",
			"_XOPEN_SOURCE=700",
		}
		buildoptions {
			"-std=c99",
			"-Winline",
			"-Wall",
			"-Wextra",
			"-fPIC",
			"-fvisibility=hidden",
			"-pthread",
		}
		includedirs {
			AL_DIR .. "OpenAL32/config_linux",
		}
		files {
			AL_DIR .. "Alc/backends/alsa.c",
			AL_DIR .. "Alc/backends/pulseaudio.c",
		}

	configuration { "osx-*" }
		defines {
			-- These are needed on non-Windows systems for extra features
			"_GNU_SOURCE=1",
			"_POSIX_C_SOURCE=200809L",
			"_XOPEN_SOURCE=700",
		}
		buildoptions {
			"-std=c99",
			"-Winline",
			"-Wall",
			"-Wextra",
			"-fPIC",
			"-fvisibility=hidden",
			"-pthread",
		}
		includedirs {
			AL_DIR .. "OpenAL32/config_linux",
		}
		files {
			AL_DIR .. "Alc/backends/alsa.c",
			AL_DIR .. "Alc/backends/pulseaudio.c",
		}		

	configuration {}

	includedirs {
		AL_DIR .. "include",
		AL_DIR .. "Alc",
		AL_DIR .. "OpenAL32/Include",
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
