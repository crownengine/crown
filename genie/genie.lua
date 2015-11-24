--
-- Copyright (c) 2012-2015 Daniele Bartolini and individual contributors.
-- License: https://github.com/taylor001/crown/blob/master/LICENSE
--

CROWN_DIR = (path.getabsolute("..") .. "/")
local CROWN_THIRD_DIR = (CROWN_DIR .. "third/")
local CROWN_BUILD_DIR = (CROWN_DIR .. "build/")
BGFX_DIR = (CROWN_DIR .. "third/bgfx/")
BX_DIR = (CROWN_DIR .. "third/bx/")

function copyLib()
end

newoption {
	trigger = "with-openal",
	description = "Build with OpenAL support."
}

newoption {
	trigger = "with-luajit",
	description = "Build with luajit support."
}

newoption {
	trigger = "with-tools",
	description = "Build with tools."
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

	configuration {}

dofile ("toolchain.lua")
dofile (BGFX_DIR .. "scripts/bgfx.lua")
dofile ("crown.lua")

toolchain(CROWN_BUILD_DIR, CROWN_THIRD_DIR)

group "libs"
bgfxProject("", "StaticLib", os.is("windows") and { "BGFX_CONFIG_RENDERER_DIRECT3D9=1" } or {})

if _OPTIONS["with-tools"] then
	dofile ("shaderc.lua")
end

if _OPTIONS["with-openal"] then
	dofile ("openal.lua")
end

group "engine"
crown_project("", "ConsoleApp", {})

-- Install
configuration { "x32", "linux-*" }
	postbuildcommands {
		"cp    " .. CROWN_THIRD_DIR .. "luajit/pre/linux_x86/luajit " .. CROWN_BUILD_DIR .. "linux32/bin",
		"cp -r " .. CROWN_THIRD_DIR .. "luajit/src/jit " ..              CROWN_BUILD_DIR .. "linux32/bin",
		"cp -r " .. "$(PHYSX_SDK_LINUX)/Bin/linux32/libPhysX3* " ..      CROWN_BUILD_DIR .. "linux32/bin",
	}

configuration { "x64", "linux-*" }
	postbuildcommands {
		"cp    " .. CROWN_THIRD_DIR .. "luajit/pre/linux_x64/luajit " .. CROWN_BUILD_DIR .. "linux64/bin",
		"cp -r " .. CROWN_THIRD_DIR .. "luajit/src/jit " ..              CROWN_BUILD_DIR .. "linux64/bin",
		"cp -r " .. "$(PHYSX_SDK_LINUX)/Bin/linux64/libPhysX3* " ..      CROWN_BUILD_DIR .. "linux64/bin",
	}

configuration { "x32", "vs*" }
	postbuildcommands {
		"cp    " .. CROWN_THIRD_DIR .. "luajit/pre/win_x86/luajit.exe " ..  CROWN_BUILD_DIR .. "win32/bin",
		"cp    " .. CROWN_THIRD_DIR .. "luajit/pre/win_x86/lua51.lib " ..   CROWN_BUILD_DIR .. "win32/bin",
		"cp    " .. CROWN_THIRD_DIR .. "luajit/pre/win_x86/lua51.exp " ..   CROWN_BUILD_DIR .. "win32/bin",
		"cp    " .. CROWN_THIRD_DIR .. "luajit/pre/win_x86/lua51.dll " ..   CROWN_BUILD_DIR .. "win32/bin",
		"cp -r " .. CROWN_THIRD_DIR .. "luajit/src/jit " ..                 CROWN_BUILD_DIR .. "win32/bin",
		"cp    " .. "$(PHYSX_SDK_WINDOWS)/bin/win32/PhysX3* " ..            CROWN_BUILD_DIR .. "win32/bin",
		"cp    " .. "$(PHYSX_SDK_WINDOWS)/bin/win32/nvToolsExt32_1.dll " .. CROWN_BUILD_DIR .. "win32/bin",
	}

configuration { "x64", "vs*" }
	postbuildcommands {
		"cp    " .. CROWN_THIRD_DIR .. "luajit/pre/win_x64/luajit.exe " ..  CROWN_BUILD_DIR .. "win64/bin",
		"cp    " .. CROWN_THIRD_DIR .. "luajit/pre/win_x64/lua51.lib " ..   CROWN_BUILD_DIR .. "win64/bin",
		"cp    " .. CROWN_THIRD_DIR .. "luajit/pre/win_x64/lua51.exp " ..   CROWN_BUILD_DIR .. "win64/bin",
		"cp    " .. CROWN_THIRD_DIR .. "luajit/pre/win_x64/lua51.dll " ..   CROWN_BUILD_DIR .. "win64/bin",
		"cp -r " .. CROWN_THIRD_DIR .. "luajit/src/jit " ..                 CROWN_BUILD_DIR .. "win64/bin",
		"cp    " .. "$(PHYSX_SDK_WINDOWS)/bin/win64/PhysX3* " ..            CROWN_BUILD_DIR .. "win64/bin",
		"cp    " .. "$(PHYSX_SDK_WINDOWS)/bin/win64/nvToolsExt64_1.dll " .. CROWN_BUILD_DIR .. "win64/bin",
	}
