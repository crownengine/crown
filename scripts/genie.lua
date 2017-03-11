--
-- Copyright (c) 2012-2017 Daniele Bartolini and individual contributors.
-- License: https://github.com/taylor001/crown/blob/master/LICENSE
--

CROWN_DIR = (path.getabsolute("..") .. "/")
local CROWN_THIRD_DIR  = (CROWN_DIR .. "3rdparty/")
local CROWN_BUILD_DIR  = (CROWN_DIR .. "build/")
BGFX_DIR = (CROWN_DIR .. "3rdparty/bgfx/")
BX_DIR = (CROWN_DIR .. "3rdparty/bx/")

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
	trigger = "with-bullet",
	description = "Build with Bullet support."
}

newoption {
	trigger = "with-tools",
	description = "Build with tools."
}

newoption {
	trigger = "no-level-editor",
	description = "Do not build Level Editor."
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

if _OPTIONS["with-openal"] then
	dofile ("openal.lua")
	openal_project(os.is("windows") and "SharedLib" or "StaticLib")
end

if _OPTIONS["with-bullet"] then
	dofile ("bullet.lua")
end

group "engine"
crown_project("", "ConsoleApp", {})

if _OPTIONS["with-tools"] then
group "tools"
	if not _OPTIONS["no-level-editor"] then
		dofile ("level-editor.lua")
	end
	dofile ("shaderc.lua")
	dofile ("texturec.lua")
end
