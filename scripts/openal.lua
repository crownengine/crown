--
-- Copyright (c) 2012-2021 Daniele Bartolini et al.
-- License: https://github.com/dbartolini/crown/blob/master/LICENSE
--

function openal_project(_kind)
	project "openal"
		kind (_kind)

		configuration {}

		local AL_DIR = (CROWN_DIR .. "3rdparty/openal/")

		defines {
			"AL_ALEXT_PROTOTYPES",
			"HAVE_MALLOC_H",
			"HAVE_STAT",
			"SIZEOF_LONG_LONG=8",
		}

		configuration { "not vs*" }
			defines {
				"HAVE_DIRENT_H",
				"HAVE_PTHREAD_SETNAME_NP",
				"HAVE_PTHREAD_SETSCHEDPARAM",
				"restrict=__restrict",
				"SIZEOF_LONG=8",
			}
			buildoptions {
				"-fPIC",
				"-fvisibility=hidden",
				"-fexceptions" -- :(
			}

		configuration { "android-* or linux-*" }
			defines {
				"HAVE_DLFCN_H",
				"HAVE_GCC_GET_CPUID",
			}
			links {
				"pthread",
			}

		configuration { "not android-*" }
			defines {
				"HAVE_SSE",
				"HAVE_SSE2",
			}
			files {
				AL_DIR .. "alc/mixer/mixer_sse2.cpp",
				AL_DIR .. "alc/mixer/mixer_sse.cpp",
			}

		configuration { "android-*" }
			defines {
				"HAVE_NEON",
				"HAVE_OPENSL",
			}
			files {
				AL_DIR .. "alc/mixer/mixer_neon.cpp",
				AL_DIR .. "alc/backends/opensl.cpp",
			}

		configuration { "linux-*" }
			defines {
				"HAVE_CPUID_H",
				"HAVE_POSIX_MEMALIGN",
				"HAVE_PULSEAUDIO",
			}
			files {
				AL_DIR .. "alc/backends/pulseaudio.cpp",
			}

		configuration { "vs* or mingw-*"}
			defines {
				"_WIN32_WINNT=0x0502",
				"_WINDOWS",
				"HAVE__ALIGNED_MALLOC",
				"HAVE_CPUID_INTRINSIC",
				"HAVE_DSOUND",
				"HAVE_GUIDDEF_H",
				"HAVE_INTRIN_H",
				"HAVE_WASAPI",
				"HAVE_WINDOWS_H",
				"HAVE_WINMM",
				"strcasecmp=_stricmp",
				"strncasecmp=_strnicmp",
			}
			files {
				AL_DIR .. "alc/backends/dsound.cpp",
				AL_DIR .. "alc/backends/wasapi.cpp",
				AL_DIR .. "alc/backends/winmm.cpp",
			}
			links {
				"winmm",
				"ole32",
			}

		configuration { "vs*" }
			defines {
				"_CRT_NONSTDC_NO_DEPRECATE",
				"restrict=",
				"SIZEOF_LONG=4",
			}
			buildoptions {
				"/wd4098",
				"/wd4267",
				"/wd4244",
				"/EHs", -- :(
			}

		configuration {}

		includedirs {
			AL_DIR .. "include",
			AL_DIR,
			AL_DIR .. "alc",
			AL_DIR .. "common",
		}

		files {
			AL_DIR .. "al/*.cpp",
			AL_DIR .. "alc/*.cpp",
			AL_DIR .. "alc/backends/base.cpp",
			AL_DIR .. "alc/backends/loopback.cpp",
			AL_DIR .. "alc/backends/null.cpp",
			AL_DIR .. "alc/effects/*.cpp",
			AL_DIR .. "alc/filters/*.cpp",
			AL_DIR .. "alc/midi/*.cpp",
			AL_DIR .. "alc/mixer/mixer_c.cpp",
			AL_DIR .. "common/*.cpp",
		}

		configuration {}
end
