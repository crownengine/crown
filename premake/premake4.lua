--
-- Copyright (c) 2012-2014 Daniele Bartolini and individual contributors.
-- License: https://github.com/taylor001/crown/blob/master/LICENSE
--

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
dofile (CROWN_DIR .. "premake/toolchain.lua")
toolchain(CROWN_BUILD_DIR, CROWN_THIRD_DIR)

	configuration { "linux-*" }
		links {
			"X11",
			"Xrandr",
			"pthread",
			"GL",
			"openal",
			"luajit",
			"dl",
		}

	configuration { "debug", "linux-*" }
		links {
			"bgfxDebug"
		}
		linkoptions {
			"-rdynamic",
			"-Wl,--start-group $(addprefix -l," ..
			"	LowLevelClothCHECKED" ..
			"	PhysX3CHECKED " ..
			"	PhysX3CommonCHECKED" ..
			"	PxTaskCHECKED" ..
			"	LowLevelCHECKED" ..
			"	PhysX3CharacterKinematicCHECKED" ..
			"	PhysX3CookingCHECKED" ..
			"	PhysX3ExtensionsCHECKED" ..
			"	PhysX3VehicleCHECKED" ..
			"	PhysXProfileSDKCHECKED" ..
			"	PhysXVisualDebuggerSDKCHECKED" ..
			"	PvdRuntimeCHECKED" ..
			"	SceneQueryCHECKED" ..
			"	SimulationControllerCHECKED" ..
			") -Wl,--end-group"
		}

	configuration { "development", "linux-*" }
		links {
			"bgfxDebug"
		}
		linkoptions
		{
			"-rdynamic",
			"-Wl,--start-group $(addprefix -l," ..
			"	LowLevelClothPROFILE" ..
			"	PhysX3PROFILE " ..
			"	PhysX3CommonPROFILE" ..
			"	PxTaskPROFILE" ..
			"	LowLevelPROFILE" ..
			"	PhysX3CharacterKinematicPROFILE" ..
			"	PhysX3CookingPROFILE" ..
			"	PhysX3ExtensionsPROFILE" ..
			"	PhysX3VehiclePROFILE" ..
			"	PhysXProfileSDKPROFILE" ..
			"	PhysXVisualDebuggerSDKPROFILE" ..
			"	PvdRuntimePROFILE" ..
			"	SceneQueryPROFILE" ..
			"	SimulationControllerPROFILE" ..
			") -Wl,--end-group"
		}

	configuration { "release", "linux-*" }
		links {
			"bgfxRelease"
		}
		linkoptions {
			"-Wl,--start-group $(addprefix -l," ..
			"	LowLevelCloth" ..
			"	PhysX3 " ..
			"	PhysX3Common" ..
			"	PxTask" ..
			"	LowLevel" ..
			"	PhysX3CharacterKinematic" ..
			"	PhysX3Cooking" ..
			"	PhysX3Extensions" ..
			"	PhysX3Vehicle" ..
			"	PhysXProfileSDK" ..
			"	PhysXVisualDebuggerSDK" ..
			"	PvdRuntime" ..
			"	SceneQuery" ..
			"	SimulationController" ..
			") -Wl,--end-group"
		}

	configuration { "android*" }
		kind "ConsoleApp"
		targetextension ".so"
		linkoptions {
			"-shared"
		}
		links {
			"EGL",
			"GLESv2",
			":libluajit.a",
			"OpenSLES",
		}

	configuration { "debug", "android-arm" }
		links {
			":libbgfxDebug.a",
		}
		linkoptions {
			"-Wl,--start-group $(addprefix -l," ..
			"	LowLevelCloth" ..
			"	PhysX3 " ..
			"	PhysX3Common" ..
			"	PxTask" ..
			"	LowLevel" ..
			"	PhysX3CharacterKinematic" ..
			"	PhysX3Cooking" ..
			"	PhysX3Extensions" ..
			"	PhysX3Vehicle" ..
			"	PhysXProfileSDK" ..
			"	PhysXVisualDebuggerSDK" ..
			"	PvdRuntime" ..
			"	SceneQuery" ..
			"	SimulationController" ..
			") -Wl,--end-group"
		}

	configuration { "development", "android-arm" }
		links {
			":libbgfxDebug.a",
		}
		linkoptions {
			"-Wl,--start-group $(addprefix -l," ..
			"	LowLevelCloth" ..
			"	PhysX3 " ..
			"	PhysX3Common" ..
			"	PxTask" ..
			"	LowLevel" ..
			"	PhysX3CharacterKinematic" ..
			"	PhysX3Cooking" ..
			"	PhysX3Extensions" ..
			"	PhysX3Vehicle" ..
			"	PhysXProfileSDK" ..
			"	PhysXVisualDebuggerSDK" ..
			"	PvdRuntime" ..
			"	SceneQuery" ..
			"	SimulationController" ..
			") -Wl,--end-group"
		}

	configuration { "release", "android-arm" }
		links {
			":libbgfxRelease.a",
		}
		linkoptions {
			"-Wl,--start-group $(addprefix -l," ..
			"	LowLevelCloth" ..
			"	PhysX3 " ..
			"	PhysX3Common" ..
			"	PxTask" ..
			"	LowLevel" ..
			"	PhysX3CharacterKinematic" ..
			"	PhysX3Cooking" ..
			"	PhysX3Extensions" ..
			"	PhysX3Vehicle" ..
			"	PhysXProfileSDK" ..
			"	PhysXVisualDebuggerSDK" ..
			"	PvdRuntime" ..
			"	SceneQuery" ..
			"	SimulationController" ..
			") -Wl,--end-group"
		}

	configuration { "vs*" }
		links {
			"OpenGL32",
			"lua51",
			"OpenAL32",
			"dbghelp"
		}

	configuration { "debug", "x32", "vs*"}
		links {
			"PhysX3CharacterKinematicCHECKED_x86",
			"PhysX3CHECKED_x86",
			"PhysX3CommonCHECKED_x86",
			"PhysX3CookingCHECKED_x86",
			"PhysX3ExtensionsCHECKED",
			"bgfxDebug"
		}

	configuration { "debug", "x64", "vs*" }
		links {
			"PhysX3CharacterKinematicCHECKED_x64",
			"PhysX3CHECKED_x64",
			"PhysX3CommonCHECKED_x64",
			"PhysX3CookingCHECKED_x64",
			"PhysX3ExtensionsCHECKED",
			"bgfxDebug"
		}

	configuration { "development", "x32", "vs*" }
		links {
			"PhysX3CharacterKinematicPROFILE_x86",
			"PhysX3PROFILE_x86",
			"PhysX3CommonPROFILE_x86",
			"PhysX3CookingPROFILE_x86",
			"PhysX3ExtensionsPROFILE",
			"bgfxDebug"
		}

	configuration { "development", "x64", "vs*" }
		links {
			"PhysX3CharacterKinematicPROFILE_x64",
			"PhysX3PROFILE_x64",
			"PhysX3CommonPROFILE_x64",
			"PhysX3CookingPROFILE_x64",
			"PhysX3ExtensionsPROFILE",
			"bgfxDebug"
		}

	configuration { "release", "x32", "vs*" }
		links {
			"PhysX3CharacterKinematic_x86",
			"PhysX3_x86",
			"PhysX3Common_x86",
			"PhysX3Cooking_x86",
			"PhysX3Extensions",
			"bgfxRelease"
		}

	configuration { "release", "x64", "vs*" }
		links {
			"PhysX3CharacterKinematic_x64",
			"PhysX3_x64",
			"PhysX3Common_x64",
			"PhysX3Cooking_x64",
			"PhysX3Extensions",
			"bgfxRelease"
		}

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
