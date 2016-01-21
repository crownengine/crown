--
-- Copyright (c) 2012-2016 Daniele Bartolini and individual contributors.
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
		"HAVE_STAT",
		"HAVE_LRINTF",
		"HAVE_STRTOF",
		"HAVE_C99_BOOL",
		"HAVE_STDINT_H",
		"HAVE_STDBOOL_H",
		"HAVE_FLOAT_H",
		"HAVE_FENV_H",
		"HAVE_MALLOC_H",
	}

	configuration { "android-* or linux-*" }
		defines {
			"'ALIGN(x)=__attribute__((aligned(x)))'",
			"restrict=__restrict",
			"SIZEOF_LONG=8",
			"SIZEOF_LONG_LONG=8",
			"HAVE_C99_VLA",
			"HAVE_GCC_DESTRUCTOR",
			"HAVE_GCC_FORMAT",
			"HAVE_DLFCN_H",
			"HAVE_STRINGS_H",
			"HAVE_PTHREAD_SETSCHEDPARAM",
			"HAVE_PTHREAD_SETNAME_NP",
			"HAVE_GCC_GET_CPUID",
			"HAVE_DIRENT_H",
		}
		buildoptions {
			"-Winline",
			"-fPIC",
			"-fvisibility=hidden",
		}

	configuration { "linux-* or vs*" }
		defines {
			"HAVE_SSE",
			"HAVE_SSE2",
		}
		files {
			AL_DIR .. "Alc/mixer_sse2.c",
			AL_DIR .. "Alc/mixer_sse.c",
		}

	configuration { "not vs*" }
		defines {
			-- These are needed on non-Windows systems for extra features
			"_GNU_SOURCE=1",
			"_POSIX_C_SOURCE=200809L",
			"_XOPEN_SOURCE=700",
		}

	configuration { "android-*" }
		files {
			AL_DIR .. "Alc/backends/opensl.c"
		}
		links {
			"OpenSLES",
		}

	configuration { "vs*" }
		defines {
			"HAVE__ALIGNED_MALLOC",
			"HAVE_MMDEVAPI",
			"HAVE_DSOUND",
			"HAVE_WINMM",
			"SIZEOF_LONG=4",
			"SIZEOF_LONG_LONG=8",
			"HAVE_WINDOWS_H",
			"HAVE_IO_H",
			"HAVE_INTRIN_H",
			"HAVE_GUIDDEF_H",
			"HAVE_CPUID_INTRINSIC",
			"HAVE__CONTROLFP",
			"HAVE___CONTROL87_2",
			"_WINDOWS",
			"_WIN32_WINNT=0x0502",
			"inline=__inline",
			"restrict=",
			"_CRT_NONSTDC_NO_DEPRECATE",
			"strcasecmp=_stricmp",
			"strncasecmp=_strnicmp",
			"snprintf=_snprintf",
		}
		buildoptions {
			"/wd4098",
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
			"HAVE_ALIGNED_ALLOC",
			"HAVE_POSIX_MEMALIGN",
			"HAVE_PULSEAUDIO",
			"HAVE_CPUID_H",
			"HAVE_PTHREAD_MUTEX_TIMEDLOCK",
		}
		files {
			AL_DIR .. "Alc/backends/pulseaudio.c",
		}

	configuration {}

	includedirs {
		AL_DIR,
		AL_DIR .. "include",
		AL_DIR .. "Alc",
		AL_DIR .. "OpenAL32/Include",
	}

	files {
		AL_DIR .. "common/*.c",
		AL_DIR .. "Alc/ALc.c",
		AL_DIR .. "Alc/ALu.c",
		AL_DIR .. "Alc/bsinc.c",
		AL_DIR .. "Alc/alcConfig.c",
		AL_DIR .. "Alc/alcRing.c",
		AL_DIR .. "Alc/bs2b.c",
		AL_DIR .. "Alc/helpers.c",
		AL_DIR .. "Alc/hrtf.c",
		AL_DIR .. "Alc/panning.c",
		AL_DIR .. "Alc/mixer.c",
		AL_DIR .. "Alc/mixer_c.c",
		AL_DIR .. "Alc/effects/*.c",
		AL_DIR .. "Alc/midi/*.c",
		AL_DIR .. "Alc/backends/base.c",
		AL_DIR .. "Alc/backends/loopback.c",
		AL_DIR .. "Alc/backends/null.c",
		AL_DIR .. "OpenAL32/*.c",
	}

	configuration {}
