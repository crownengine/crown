--
-- Copyright (c) 2012-2015 Daniele Bartolini and individual contributors.
-- License: https://github.com/taylor001/crown/blob/master/LICENSE
--

CROWN_DIR = (path.getabsolute("..") .. "/")
local CROWN_THIRD_DIR = (CROWN_DIR .. "third/")
local CROWN_BUILD_DIR = (CROWN_DIR .. ".build/")
BGFX_DIR = (CROWN_DIR .. "third/bgfx/")
BX_DIR = (CROWN_DIR .. "third/bx/")

function copyLib()
end

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

	configuration {}

dofile ("toolchain.lua")
dofile (BGFX_DIR .. "scripts/bgfx.lua")
dofile ("crown.lua")

toolchain(CROWN_BUILD_DIR, CROWN_THIRD_DIR)

group "libs"
bgfxProject("", "StaticLib", os.is("windows") and { "BGFX_CONFIG_RENDERER_DIRECT3D9=1" } or {})
dofile ("shaderc.lua")

if _OPTIONS["with-openal"] then
	dofile ("openal.lua")
end

group "engine"
crown_project("", "ConsoleApp", {})


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
		"cp    " .. CROWN_THIRD_DIR .. "luajit/src/lua51.lib " .. "$(CROWN_INSTALL_DIR)/" .. "bin/win32",
		"cp    " .. CROWN_THIRD_DIR .. "luajit/src/lua51.exp " .. "$(CROWN_INSTALL_DIR)/" .. "bin/win32",
		"cp    " .. CROWN_THIRD_DIR .. "luajit/src/lua51.dll " .. "$(CROWN_INSTALL_DIR)/" .. "bin/win32",
		"cp    " .. "$(PHYSX_SDK_WINDOWS)/bin/win32/PhysX3* " .. "$(CROWN_INSTALL_DIR)/" .. "bin/win32",
		"cp    " .. "$(PHYSX_SDK_WINDOWS)/bin/win32/nvToolsExt32_1.dll " .. "$(CROWN_INSTALL_DIR)/" .. "bin/win32",
	}

configuration { "x64", "vs*" }
	postbuildcommands {
		"cp -r " .. CROWN_DIR .. ".build/win64/bin/* " .. "$(CROWN_INSTALL_DIR)/" .. "bin/win64/",
		"cp    " .. CROWN_THIRD_DIR .. "luajit/src/luajit.exe " .. "$(CROWN_INSTALL_DIR)/" .. "bin/win64/",
		"cp -r " .. CROWN_THIRD_DIR .. "luajit/src/jit " .. "$(CROWN_INSTALL_DIR)/" .. "bin/win64/",
		"cp    " .. CROWN_THIRD_DIR .. "luajit/src/lua51.lib " .. "$(CROWN_INSTALL_DIR)/" .. "bin/win64",
		"cp    " .. CROWN_THIRD_DIR .. "luajit/src/lua51.exp " .. "$(CROWN_INSTALL_DIR)/" .. "bin/win64",
		"cp    " .. CROWN_THIRD_DIR .. "luajit/src/lua51.dll " .. "$(CROWN_INSTALL_DIR)/" .. "bin/win64",
		"cp    " .. "$(PHYSX_SDK_WINDOWS)/bin/win64/PhysX3* " .. "$(CROWN_INSTALL_DIR)/" .. "bin/win64",
		"cp    " .. "$(PHYSX_SDK_WINDOWS)/bin/win64/nvToolsExt64_1.dll " .. "$(CROWN_INSTALL_DIR)/" .. "bin/win64",
	}
