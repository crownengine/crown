--
-- Copyright (c) 2012-2026 Daniele Bartolini et al.
-- SPDX-License-Identifier: MIT
--

local LUA_DIR = (CROWN_DIR .. "3rdparty/lua/")

project "luac"
	kind "ConsoleApp"
	language "C"

	links {
		"lua"
	}

	configuration { "linux-*" }
		links {
			"m"
		}

	configuration { }

	files {
		LUA_DIR .. "src/luac.c",
	}
