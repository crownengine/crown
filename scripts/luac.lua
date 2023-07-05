--
-- Copyright (c) 2012-2023 Daniele Bartolini et al.
-- SPDX-License-Identifier: MIT
--

local LUA_DIR = (CROWN_DIR .. "3rdparty/lua/")

project "luac"
	kind "ConsoleApp"
	language "C"

	configuration { "linux-*" }
		links {
			"m"
		}

	links {
		"lua"
	}

	files {
		LUA_DIR .. "src/luac.c",
	}
