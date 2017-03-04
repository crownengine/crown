--
-- Copyright (c) 2012-2017 Daniele Bartolini and individual contributors.
-- License: https://github.com/taylor001/crown/blob/master/LICENSE
--

solution "tools"
language "Vala"

configurations {
	"debug",
	"release",
}

local CROWN_DIR = (path.getabsolute("..") .. "/")
local CROWN_BUILD_DIR = (CROWN_DIR .. "build/")
local CROWN_TOOLS_DIR = (CROWN_BUILD_DIR .. "tools/")

newoption
{
	trigger = "compiler",
	value = "COMPILER",
	description = "Choose compiler",
	allowed =
	{
		{ "linux-gcc", "Linux (GCC compiler)" },
		{ "mingw",     "MinGW"                },
	}
}

if _ACTION == "gmake" then

	if nil == _OPTIONS["compiler"] then
		print("Choose a compiler!")
		os.exit(1)
	end

	if "linux-gcc" == _OPTIONS["compiler"] then

		if not os.is("linux") then
			print("Action not valid in current OS.")
		end

		location(CROWN_TOOLS_DIR .. "projects/" .. "linux")

	elseif "mingw" == _OPTIONS["compiler"] then

		location(CROWN_TOOLS_DIR .. "projects/" .. "mingw")
	end
else
	print("Invalid action.")
	os.exit(1)
end

	-- FIXME: Fix this in GENie
	premake.valac.valac = premake.valac.valac .. " --gresources=" .. CROWN_DIR .. "tools/ui/resources.xml" .. " --target-glib=2.38"

	configuration { "linux-*" }
		targetdir (CROWN_TOOLS_DIR .. "linux64" .. "/bin")
		objdir (CROWN_TOOLS_DIR .. "linux64" .. "/obj")

	configuration { "mingw" }
		targetdir (CROWN_TOOLS_DIR .. "mingw64" .. "/bin")
		objdir (CROWN_TOOLS_DIR .. "mingw64" .. "/obj")

	configuration { "debug" }
		targetsuffix "-debug64"
	configuration { "release" }
		targetsuffix "-release64"

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
			CROWN_DIR .. "tools/**.vala",
			CROWN_DIR .. "tools/ui/resources.c",
		}

	configuration {}
