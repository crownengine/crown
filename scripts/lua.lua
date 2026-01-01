--
-- Copyright (c) 2012-2026 Daniele Bartolini et al.
-- SPDX-License-Identifier: MIT
--

local LUA_DIR = (CROWN_DIR .. "3rdparty/lua/")

project "lua"
	kind "StaticLib"
	language "C"

	configuration { "linux-*" }
		buildoptions {
			"-Wno-misleading-indentation",
			"-Wno-implicit-fallthrough",
		}

	configuration { "wasm" }
		buildoptions {
			"-Wno-empty-body",
		}

	configuration { }

	files {
		LUA_DIR .. "src/**.c",
	}

	removefiles {
		LUA_DIR .. "src/lua.c",
		LUA_DIR .. "src/luac.c",
	}
