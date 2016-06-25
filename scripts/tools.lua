--
-- Copyright (c) 2012-2016 Daniele Bartolini and individual contributors.
-- License: https://github.com/taylor001/crown/blob/master/LICENSE
--

local CROWN_DIR = (path.getabsolute("..") .. "/")
local CROWN_BUILD_DIR = (CROWN_DIR .. "build/")
local CROWN_TOOLS_DIR = (CROWN_BUILD_DIR .. "tools/")

solution "tools"
	language "Vala"

	configurations {
		"debug",
		"release",
	}

	location(CROWN_TOOLS_DIR .. "projects/")
	targetdir(CROWN_TOOLS_DIR)

	configuration { "debug" }
		targetsuffix "-debug"

	project "level-editor"
		kind "ConsoleApp"

		configuration { "debug" }
			flags {
				"Symbols",
			}
			defines {
				"CROWN_DEBUG",
			}

		configuration { "release" }
			flags {
				"Optimize"
			}

		configuration { "linux" }
			defines {
				"CROWN_PLATFORM_LINUX"
			}

		configuration { "windows" }
			defines {
				"CROWN_PLATFORM_WINDOWS"
			}

		configuration {}

		links {
			"gdk-3.0",
			"gee-0.8",
			"gio-2.0",
			"glib-2.0",
			"gtk+-3.0",
			"posix",
		}

		buildoptions {
			"-lm",
			"-Wno-deprecated-declarations",
			"-Wno-incompatible-pointer-types",
			"-Wno-discarded-qualifiers",
		}

		files {
			CROWN_DIR .. "tools/**.vala"
		}

	configuration {}
