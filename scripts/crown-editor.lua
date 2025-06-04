--
-- Copyright (c) 2012-2026 Daniele Bartolini et al.
-- SPDX-License-Identifier: MIT
--

project "crown-editor"
	kind "ConsoleApp"
	language "Vala"

	configuration { "debug" }
		defines {
			"CROWN_DEBUG",
		}

	configuration { "linux" }
		defines {
			"CROWN_PLATFORM_LINUX"
		}
		linkoptions {
			"-lX11",
			"-lXi",
			"-lXfixes",
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
		if _OPTIONS["with-gtk4"] then
			links { "gtk4-x11" }
		end

	configuration { "mingw*" }
		targetdir (CROWN_BUILD_DIR .. "mingw64" .. "/bin")
		objdir (CROWN_BUILD_DIR .. "mingw64" .. "/obj")
		linkoptions {
			"-lgdi32", -- gtkcolorpickerwin32.c
		}

	configuration {}

	prebuildcommands {
		'make -C "' .. path.join(CROWN_DIR, "tools/po") .. '" TARGETDIR="$(abspath $(TARGETDIR))"'
	}

	flags {
		"FatalWarnings"
	}

	removelinkoptions {
		"-static"
	}
	removelinks {
		"dl"
	}
	if _OPTIONS["with-gtk4"] then
		links {
			"gio-2.0",
			"glib-2.0",
			"gtk4",
		}
	else
		defines {
			"CROWN_GTK3"
		}
		buildoptions {
			"-D CROWN_GTK3",
		}
		links {
			"gdk-3.0",
			"gio-2.0",
			"glib-2.0",
			"gtk+-3.0",
		}
	end

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
		"-Wno-address",
		"-D VALA_STRICT_C",
		"-D GDK_DISABLE_DEPRECATED",
		"-D GTK_DISABLE_DEPRECATED",
		"-DGETTEXT_PACKAGE=\\\"crown-editor\\\"",
	}

	linkoptions {
		"-lm",
		"-lfreetype",
	}

	configuration {}

	buildoptions_vala {
		"--target-glib=2.64.6",
		"--pkg tinyexpr",
		"--pkg md5",
		"--pkg ufbx",
	}
	if _OPTIONS["with-gtk4"] then
		buildoptions_vala {
			"--disable-warnings",
		}
	end

	vapidirs {
		CROWN_DIR .. "tools/vapi"
	}

	buildoptions {
		'-I"' .. path.join(CROWN_DIR, "3rdparty/tinyexpr") .. '"',
		'-I"' .. path.join(CROWN_DIR, "3rdparty/md5") .. '"',
		'-I"' .. path.join(CROWN_DIR, "3rdparty/ufbx") .. '"',
		'-I"' .. path.join(CROWN_DIR, "3rdparty/stb") .. '"',
	}

	files {
		CROWN_DIR .. "tools/**.vala",
		CROWN_DIR .. "tools/**.c",
		CROWN_DIR .. "tools/level_editor/resources/org.crownengine.Crown.gresource.xml",
		CROWN_DIR .. "3rdparty/tinyexpr/tinyexpr.c",
		CROWN_DIR .. "3rdparty/md5/md5.c",
		CROWN_DIR .. "3rdparty/ufbx/ufbx.c",
		CROWN_DIR .. "scripts/win32/app.rc",
	}

	configuration { "linux" }
		removefiles {
			CROWN_DIR .. "tools/widgets/gtk/gtkcolorpickerquartz.c",
			CROWN_DIR .. "tools/widgets/gtk/gtkcolorpickerwin32.c",
			CROWN_DIR .. "scripts/win32/app.rc",
		}
	configuration { "windows" }
		removefiles {
			CROWN_DIR .. "tools/widgets/gtk/gtkcolorpickerkwin.c",
			CROWN_DIR .. "tools/widgets/gtk/gtkcolorpickerquartz.c",
			CROWN_DIR .. "tools/widgets/gtk/gtkcolorpickershell.c",
		}

	strip()

	configuration {}
