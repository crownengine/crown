--
-- Copyright (c) 2012-2026 Daniele Bartolini et al.
-- SPDX-License-Identifier: MIT
--

project "crown-editor"
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
		buildoptions_vala { "--pkg posix" }

	configuration { "mingw*" }
		targetdir (CROWN_BUILD_DIR .. "mingw64" .. "/bin")
		objdir (CROWN_BUILD_DIR .. "mingw64" .. "/obj")

	configuration {}

	defines {
		"CROWN_GTK3"
	}

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
		"-Wno-incompatible-pointer-types",
		"-Wno-unused-value",
		"-D VALA_STRICT_C",
		"-D GDK_DISABLE_DEPRECATED",
		"-D GTK_DISABLE_DEPRECATED",
	}

	linkoptions {
		"-lm",
		"-lfreetype",
	}

	buildoptions_vala {
		"--target-glib=2.64.6",
		"--pkg tinyexpr",
		"--pkg md5",
		"--pkg ufbx",
	}

	vapidirs {
		CROWN_DIR .. "tools/vapi"
	}

	buildoptions {
		"-I" .. CROWN_DIR .. "3rdparty/tinyexpr",
		"-I" .. CROWN_DIR .. "3rdparty/md5",
		"-I" .. CROWN_DIR .. "3rdparty/ufbx",
		"-I" .. CROWN_DIR .. "3rdparty/stb",
	}

	files {
		CROWN_DIR .. "tools/**.vala",
		CROWN_DIR .. "tools/**.c",
		CROWN_DIR .. "tools/level_editor/resources/org.crownengine.Crown.gresource.xml",
		CROWN_DIR .. "3rdparty/tinyexpr/tinyexpr.c",
		CROWN_DIR .. "3rdparty/md5/md5.c",
		CROWN_DIR .. "3rdparty/ufbx/ufbx.c",
	}

	strip()

	configuration {}
