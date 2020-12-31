--
-- Copyright (c) 2012-2021 Daniele Bartolini et al.
-- License: https://github.com/dbartolini/crown/blob/master/LICENSE
--

project "level-editor-imgui"
	kind "WindowedApp"
	language "C++"

	includedirs {
		CROWN_DIR .. "src",
		CROWN_DIR .. "tools-imgui",
		CROWN_DIR .. "3rdparty",
		CROWN_DIR .. "3rdparty/bgfx/include",
		CROWN_DIR .. "3rdparty/bx/include",
		CROWN_DIR .. "3rdparty/stb",
		CROWN_DIR .. "3rdparty/luajit/src",
		CROWN_DIR .. "3rdparty/ocornut-imgui",
		CROWN_DIR .. "3rdparty/nativefiledialog/src/include",
	}

	defines {
		"CROWN_TOOLS=1",
		"CROWN_PHYSICS_NOOP=1",
		"CROWN_SOUND_NOOP=1",
	}

	links {
		"bgfx",
		"bimg",
		"bx",
	}

	configuration { "debug or development" }
		defines {
			"CROWN_DEBUG=1"
		}

	configuration { "development" }
		defines {
			"CROWN_DEVELOPMENT=1"
		}

	configuration { "linux-*" }
		links {
			"X11",
			"Xrandr",
			"pthread",
			"dl",
			"GL",
			"luajit",
		}
		linkoptions { "`pkg-config --libs gtk+-3.0`" }
		buildoptions { "`pkg-config --cflags gtk+-3.0`" }
		files {
			CROWN_DIR .. "3rdparty/nativefiledialog/src/nfd_gtk.c",
		}

	configuration { "vs* or mingw*" }
		links {
			"dbghelp",
			"xinput",
			"psapi",
			"ws2_32",
			"ole32",
			"gdi32",
			"uuid",
		}
		files {
			CROWN_DIR .. "3rdparty/nativefiledialog/src/nfd_win.cpp",
		}
	configuration { "not vs*" }
		links {
			"luajit"
		}
	configuration { "vs*"}
		links {
			"lua51"
		}

	configuration {}

	files {
		CROWN_DIR .. "src/**.h",
		CROWN_DIR .. "src/**.cpp",
		CROWN_DIR .. "tools-imgui/**.h",
		CROWN_DIR .. "tools-imgui/**.cpp",
		CROWN_DIR .. "3rdparty/ocornut-imgui/*.h",
		CROWN_DIR .. "3rdparty/ocornut-imgui/*.cpp",
		CROWN_DIR .. "3rdparty/nativefiledialog/src/nfd_common.c",
	}

	configuration {} -- reset configuration
