--
-- Copyright (c) 2012-2017 Daniele Bartolini and individual contributors.
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
		CROWN_DIR .. "3rdparty/openal/include",
		CROWN_DIR .. "3rdparty/bullet3/src",
		CROWN_DIR .. "3rdparty/ocornut-imgui",
	}

	defines {
		"CROWN_TOOLS=1",
		"CROWN_PHYSICS_NOOP=1",
	}

	links {
		"bgfx",
		"bimg",
		"bx",
		"openal",
		"bullet",
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

	configuration { "vs* or mingw*" }
		links {
			"dbghelp",
			"xinput",
			"psapi",
			"ws2_32",
			"ole32",
			"gdi32",
		}
	configuration { "not vs*" }
		links {
			"luajit"
		}
	configuration { "vs*"}
		links {
			"lua51"
		}
	configuration { "x32", "vs*" }
		libdirs {
			CROWN_DIR .. "3rdparty/luajit/pre/win_x32"
		}
	configuration { "x64", "vs*" }
		libdirs {
			CROWN_DIR .. "3rdparty/luajit/pre/win_x64"
		}

	configuration {}

	files {
		CROWN_DIR .. "src/**.h",
		CROWN_DIR .. "src/**.cpp",
		CROWN_DIR .. "tools-imgui/**.h",
		CROWN_DIR .. "tools-imgui/**.cpp",
		CROWN_DIR .. "3rdparty/ocornut-imgui/*.h",
		CROWN_DIR .. "3rdparty/ocornut-imgui/*.cpp",
	}

	configuration {} -- reset configuration
