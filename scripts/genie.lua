--
-- Copyright (c) 2012-2021 Daniele Bartolini et al.
-- License: https://github.com/dbartolini/crown/blob/master/LICENSE
--

CROWN_DIR = (path.getabsolute("..") .. "/")
local CROWN_THIRD_DIR  = (CROWN_DIR .. "3rdparty/")
local CROWN_BUILD_DIR  = (CROWN_DIR .. "build/")
BGFX_DIR = (CROWN_DIR .. "3rdparty/bgfx/")
BIMG_DIR = (CROWN_DIR .. "3rdparty/bimg/")
BX_DIR = (CROWN_DIR .. "3rdparty/bx/")

function copyLib()
end

newoption {
	trigger = "with-luajit",
	description = "Build with luajit support."
}

newoption {
	trigger = "with-tools",
	description = "Build with tools."
}

newoption {
	trigger = "no-level-editor",
	description = "Do not build Level Editor."
}

newoption {
	trigger = "with-level-editor-imgui",
	description = "Build with Level Editor (IMGUI)."
}

newoption {
	trigger = "gfxapi",
	description = "BGFX renderer (gl32, gles2, d3d11)."
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
toolchain(CROWN_BUILD_DIR, CROWN_THIRD_DIR)

dofile ("crown.lua")
group "engine"
crown_project("", "WindowedApp", {})

group "libs"
dofile (BGFX_DIR .. "scripts/bgfx.lua")

if _OPTIONS["gfxapi"] == "gl32" then
	bgfxProject("", "StaticLib", "BGFX_CONFIG_RENDERER_OPENGL=32")
elseif _OPTIONS["gfxapi"] == "gles2" then
	bgfxProject("", "StaticLib", "BGFX_CONFIG_RENDERER_OPENGLES=20")
elseif _OPTIONS["gfxapi"] == "d3d11" then
	bgfxProject("", "StaticLib", "BGFX_CONFIG_RENDERER_DIRECT3D11=1")
else
	bgfxProject("", "StaticLib")
end

dofile (BX_DIR .. "scripts/bx.lua")
dofile (BIMG_DIR .. "scripts/bimg.lua")
dofile (BIMG_DIR .. "scripts/bimg_encode.lua")
dofile (BIMG_DIR .. "scripts/bimg_decode.lua")

dofile ("openal.lua")
openal_project(os.is("windows") and "SharedLib" or "StaticLib")

dofile ("bullet.lua")

if _OPTIONS["with-tools"] then
	group "tools"

	if not _OPTIONS["no-level-editor"] then
		dofile ("level-editor.lua")
	end

	if _OPTIONS["with-level-editor-imgui"] then
		dofile ("level-editor-imgui.lua")
	end
end
