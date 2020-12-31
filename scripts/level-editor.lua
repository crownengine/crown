--
-- Copyright (c) 2012-2021 Daniele Bartolini et al.
-- License: https://github.com/dbartolini/crown/blob/master/LICENSE
--

project "level-editor"
	kind "WindowedApp"
	language "Vala"

	configuration { "debug" }
		defines {
			"CROWN_DEBUG",
		}

	configuration { "linux" }
		defines {
			"CROWN_PLATFORM_LINUX"
		}

	configuration { "windows" }
		defines {
			"CROWN_PLATFORM_WINDOWS"
		}

	local CROWN_BUILD_DIR = (CROWN_DIR .. "build/")

	configuration { "linux-*" }
		targetdir (CROWN_BUILD_DIR .. "linux64" .. "/bin")
		objdir (CROWN_BUILD_DIR .. "linux64" .. "/obj")

	configuration { "mingw*" }
		targetdir (CROWN_BUILD_DIR .. "mingw64" .. "/bin")
		objdir (CROWN_BUILD_DIR .. "mingw64" .. "/obj")

	configuration {}

	removelinkoptions {
		"-static"
	}
	removelinks {
		"dl"
	}
	links {
		"gdk-3.0",
		"gee-0.8",
		"gio-2.0",
		"glib-2.0",
		"gtk+-3.0",
	}

	buildoptions {
		"`pkg-config --cflags gdk-3.0 gee-0.8 gio-2.0 glib-2.0 gtk+-3.0`",
		"-Wno-deprecated-declarations",
		"-Wno-incompatible-pointer-types",
		"-Wno-discarded-qualifiers",
		"-Wno-sign-compare",
		"-Wno-unused-function",
		"-Wno-unused-label",
		"-Wno-unused-variable",
		"-Wno-unused-parameter",
		"-Wno-unused-but-set-variable",
		"-Wno-cast-function-type",
		"-Wno-missing-field-initializers",
	}

	linkoptions {
		"`pkg-config --libs gdk-3.0 gee-0.8 gio-2.0 glib-2.0 gtk+-3.0`",
		"-lm"
	}

	buildoptions_vala {
		"--target-glib=2.38",
	}

	files {
		CROWN_DIR .. "tools/**.vala",
		CROWN_DIR .. "tools/level_editor/resources/resources.gresource.xml",
	}

	strip()

	configuration {}
