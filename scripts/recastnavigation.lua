--
-- Copyright (c) 2012-2026 Daniele Bartolini et al.
-- SPDX-License-Identifier: MIT
--

local RECAST_DIR = (CROWN_DIR .. "3rdparty/recastnavigation/")

project "recastnavigation"
	kind "StaticLib"
	language "C++"

	includedirs {
		RECAST_DIR .. "Detour/Include",
		RECAST_DIR .. "DetourCrowd/Include",
		RECAST_DIR .. "Recast/Include",
		RECAST_DIR .. "DebugUtils/Include",
	}

	configuration {}

	defines {
		"DT_POLYREF64=0",
	}

	configuration { "linux-*" }
		buildoptions {
			"-Wno-unused-parameter",
			"-Wno-unused-variable",
			"-Wno-unused-function",
			"-Wno-sign-compare",
			"-Wno-type-limits",
			"-Wno-parentheses",
		}

	configuration { "vs*" }
		buildoptions {
			"/wd4267",
			"/wd4244",
			"/wd4305",
		}

	configuration {}

	files {
		RECAST_DIR .. "Detour/Source/**.cpp",
		RECAST_DIR .. "DetourCrowd/Source/**.cpp",
		RECAST_DIR .. "Recast/Source/**.cpp",
		RECAST_DIR .. "DebugUtils/Source/**.cpp",
	}

	configuration {}
