--
-- Copyright (c) 2012-2026 Daniele Bartolini et al.
-- SPDX-License-Identifier: MIT
--

project ("crown-launcher")
	kind "WindowedApp"

	defines {
		"CROWN_LOG_TO_CONSOLE=0"
	}

	includedirs {
		CROWN_DIR .. "src",
		CROWN_DIR .. "3rdparty/stb",
	}

	configuration { "debug" }
		defines {
			"CROWN_DEBUG=1"
		}

	configuration { "release" }
		defines {
			"CROWN_DEBUG=0"
		}

	configuration { "linux-*" }
		links {
			"pthread",
		}

	configuration { "vs* or mingw*" }
		links {
			"dbghelp",
		}

	configuration {}

	files {
		CROWN_DIR .. "src/core/debug/**.cpp",
		CROWN_DIR .. "src/core/error/**.cpp",
		CROWN_DIR .. "src/core/memory/globals.cpp",
		CROWN_DIR .. "src/core/os.cpp",
		CROWN_DIR .. "src/core/process.cpp",
		CROWN_DIR .. "src/core/thread/mutex.cpp",
		CROWN_DIR .. "src/device/log.cpp",
		CROWN_DIR .. "tools/launcher/launcher.cpp",
	}

	strip()

	configuration {} -- reset configuration

