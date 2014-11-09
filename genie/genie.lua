--
-- Copyright (c) 2012-2014 Daniele Bartolini and individual contributors.
-- License: https://github.com/taylor001/crown/blob/master/LICENSE
--

newoption {
	trigger = "with-openal",
	description = "Build with OpenAL support."
}

solution "crown"
	configurations {
		"debug",
		"development",
		"release",
	}

	platforms {
		"x32",
		"x64",
		"native"
	}

	language "C++"

CROWN_DIR = (path.getabsolute("..") .. "/")
local CROWN_THIRD_DIR = (CROWN_DIR .. "third/")
local CROWN_BUILD_DIR = (CROWN_DIR .. ".build/")
dofile ("toolchain.lua")
toolchain(CROWN_BUILD_DIR, CROWN_THIRD_DIR)

if _OPTIONS["with-openal"] then
	dofile ("openal.lua")
end

dofile "crown.lua"
crown_project("", "WindowedApp", {})

-- Install
configuration { "android-arm" }
	postbuildcommands {
		"cp -r " .. CROWN_DIR .. ".build/android-arm/bin/* " .. "$(CROWN_INSTALL_DIR)/" .. "bin/android-arm/",
	}

configuration { "x32", "linux-*" }
	postbuildcommands {
		"cp -r " .. CROWN_DIR .. ".build/linux32/bin/* " .. "$(CROWN_INSTALL_DIR)/" .. "bin/linux32/",
		"cp    " .. CROWN_THIRD_DIR .. "luajit/src/luajit " .. "$(CROWN_INSTALL_DIR)/" .. "bin/linux32/",
		"cp -r " .. CROWN_THIRD_DIR .. "luajit/src/jit " .. "$(CROWN_INSTALL_DIR)/" .. "bin/linux32/",
	}

configuration { "x64", "linux-*" }
	postbuildcommands {
		"cp -r " .. CROWN_DIR .. ".build/linux64/bin/* " .. "$(CROWN_INSTALL_DIR)/" .. "bin/linux64/",
		"cp    " .. CROWN_THIRD_DIR .. "luajit/src/luajit " .. "$(CROWN_INSTALL_DIR)/" .. "bin/linux64/",
		"cp -r " .. CROWN_THIRD_DIR .. "luajit/src/jit " .. "$(CROWN_INSTALL_DIR)/" .. "bin/linux64/",
	}

configuration { "x32", "vs*" }
	postbuildcommands {
		"cp -r " .. CROWN_DIR .. ".build/win32/bin/* " .. "$(CROWN_INSTALL_DIR)/" .. "bin/win32/",
		"cp    " .. CROWN_THIRD_DIR .. "luajit/src/luajit.exe " .. "$(CROWN_INSTALL_DIR)/" .. "bin/win32/",
		"cp -r " .. CROWN_THIRD_DIR .. "luajit/src/jit " .. "$(CROWN_INSTALL_DIR)/" .. "bin/win32/",
	}

configuration { "x64", "vs*" }
	postbuildcommands {
		"cp -r " .. CROWN_DIR .. ".build/win64/bin/* " .. "$(CROWN_INSTALL_DIR)/" .. "bin/win64/",
		"cp    " .. CROWN_THIRD_DIR .. "luajit/src/luajit.exe " .. "$(CROWN_INSTALL_DIR)/" .. "bin/win64/",
		"cp -r " .. CROWN_THIRD_DIR .. "luajit/src/jit " .. "$(CROWN_INSTALL_DIR)/" .. "bin/win64/",
	}

configuration { "debug or development", "x32", "linux-*" }
	postbuildcommands {
		"cp " .. CROWN_THIRD_DIR .. "bgfx/.build/linux32_gcc/bin/shadercDebug " .. "$(CROWN_INSTALL_DIR)/" .. "bin/linux32"
	}

configuration { "release", "x32", "linux-*" }
	postbuildcommands {
		"cp " .. CROWN_THIRD_DIR .. "bgfx/.build/linux32_gcc/bin/shadercRelease " .. "$(CROWN_INSTALL_DIR)/" .. "bin/linux32"
	}

configuration { "debug or development", "x64", "linux-*" }
	postbuildcommands {
		"cp " .. CROWN_THIRD_DIR .. "bgfx/.build/linux64_gcc/bin/shadercDebug " .. "$(CROWN_INSTALL_DIR)/" .. "bin/linux64"
	}

configuration { "release", "x64", "linux-*" }
	postbuildcommands {
		"cp " .. CROWN_THIRD_DIR .. "bgfx/.build/linux64_gcc/bin/shadercRelease " .. "$(CROWN_INSTALL_DIR)/" .. "bin/linux64"
	}
