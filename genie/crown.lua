--
-- Copyright (c) 2012-2015 Daniele Bartolini and individual contributors.
-- License: https://github.com/taylor001/crown/blob/master/LICENSE
--

function crown_project(_name, _kind, _defines)

	project ("crown" .. _name)
		kind (_kind)

		includedirs {
			CROWN_DIR .. "engine",
			CROWN_DIR .. "engine/core",
			CROWN_DIR .. "engine/core/containers",
			CROWN_DIR .. "engine/core/filesystem",
			CROWN_DIR .. "engine/core/json",
			CROWN_DIR .. "engine/core/math",
			CROWN_DIR .. "engine/core/memory",
			CROWN_DIR .. "engine/core/network",
			CROWN_DIR .. "engine/core/settings",
			CROWN_DIR .. "engine/core/strings",
			CROWN_DIR .. "engine/core/thread",
			CROWN_DIR .. "engine/main",
			CROWN_DIR .. "engine/input",
			CROWN_DIR .. "engine/renderers",
			CROWN_DIR .. "engine/resource",
			CROWN_DIR .. "engine/lua",
			CROWN_DIR .. "engine/audio",
			CROWN_DIR .. "engine/compilers",
			CROWN_DIR .. "engine/physics",
			CROWN_DIR .. "engine/world",
			CROWN_DIR .. "third/freetype",
			CROWN_DIR .. "third/stb_image",
			CROWN_DIR .. "third/stb_vorbis",
			CROWN_DIR .. "third/bgfx/src",
			CROWN_DIR .. "third/bgfx/include",
			CROWN_DIR .. "third/bx/include",
		}

		defines {
			_defines,
		}

		links {
			"bgfx"
		}

		if _OPTIONS["with-luajit"] then
			includedirs {
				CROWN_DIR .. "third/luajit/src",
			}
			configuration { "android-arm" }
				libdirs {
					CROWN_DIR .. "third/luajit/pre/android_arm"
				}
			configuration { "linux-* or android-arm" }
				links {
					"luajit"
				}
			configuration { "x32", "linux-*" }
				libdirs {
					CROWN_DIR .. "third/luajit/pre/linux_x86"
				}
			configuration { "x64", "linux-*" }
				libdirs {
					CROWN_DIR .. "third/luajit/pre/linux_x64"
				}

			configuration { "vs*"}
				links {
					"lua51"
				}
			configuration { "x32", "vs*" }
				libdirs {
					CROWN_DIR .. "third/luajit/pre/win_x86"
				}
			configuration { "x64", "vs*" }
				libdirs {
					CROWN_DIR .. "third/luajit/pre/win_x64"
				}

				configuration {}
		end

		if _OPTIONS["with-openal"] then
			includedirs {
				CROWN_DIR .. "third/openal/include"
			}

			-- Fix this in GENie
			configuration { "debug", "x32", "linux-*" }
				linkoptions { "-Lbin/debug", "-lopenal-debug-32", }
			configuration { "development", "x32", "linux-*" }
				linkoptions { "-Lbin/debug", "-lopenal-development-32", }
			configuration { "release", "x32", "linux-*" }
				linkoptions { "-Lbin/debug", "-lopenal-release-32", }
			configuration { "debug", "x64", "linux-*" }
				linkoptions { "-Lbin/debug", "-lopenal-debug-64", }
			configuration { "development", "x64", "linux-*" }
				linkoptions { "-Lbin/debug", "-lopenal-development-64", }
			configuration { "release", "x64", "linux-*" }
				linkoptions { "-Lbin/debug", "-lopenal-release-64", }

			configuration { "vs*" }
				links { "openal", }

			configuration {}
		end

		configuration { "debug or development" }
			flags {
				"Symbols"
			}
			defines {
				"_DEBUG",
				"CROWN_DEBUG=1"
			}

		configuration { "release" }
			defines {
				"NDEBUG"
			}

		configuration { "linux*" }
			includedirs {
				"$(PHYSX_SDK_LINUX)/Include",
				"$(PHYSX_SDK_LINUX)/Include/common",
				"$(PHYSX_SDK_LINUX)/Include/characterkinematic",
				"$(PHYSX_SDK_LINUX)/Include/cloth",
				"$(PHYSX_SDK_LINUX)/Include/common",
				"$(PHYSX_SDK_LINUX)/Include/cooking",
				"$(PHYSX_SDK_LINUX)/Include/extensions",
				"$(PHYSX_SDK_LINUX)/Include/foundation",
				"$(PHYSX_SDK_LINUX)/Include/geometry",
				"$(PHYSX_SDK_LINUX)/Include/particles",
				"$(PHYSX_SDK_LINUX)/Include/physxprofilesdk",
				"$(PHYSX_SDK_LINUX)/Include/physxvisualdebuggersdk",
				"$(PHYSX_SDK_LINUX)/Include/pvd",
				"$(PHYSX_SDK_LINUX)/Include/pxtask",
				"$(PHYSX_SDK_LINUX)/Include/RepX",
				"$(PHYSX_SDK_LINUX)/Include/RepXUpgrader",
				"$(PHYSX_SDK_LINUX)/Include/vehicle",
			}

		configuration { "android*" }
			includedirs {
				"$(PHYSX_SDK_ANDROID)/Include",
				"$(PHYSX_SDK_ANDROID)/Include/common",
				"$(PHYSX_SDK_ANDROID)/Include/characterkinematic",
				"$(PHYSX_SDK_ANDROID)/Include/cloth",
				"$(PHYSX_SDK_ANDROID)/Include/common",
				"$(PHYSX_SDK_ANDROID)/Include/cooking",
				"$(PHYSX_SDK_ANDROID)/Include/extensions",
				"$(PHYSX_SDK_ANDROID)/Include/foundation",
				"$(PHYSX_SDK_ANDROID)/Include/geometry",
				"$(PHYSX_SDK_ANDROID)/Include/particles",
				"$(PHYSX_SDK_ANDROID)/Include/physxprofilesdk",
				"$(PHYSX_SDK_ANDROID)/Include/physxvisualdebuggersdk",
				"$(PHYSX_SDK_ANDROID)/Include/pvd",
				"$(PHYSX_SDK_ANDROID)/Include/pxtask",
				"$(PHYSX_SDK_ANDROID)/Include/RepX",
				"$(PHYSX_SDK_ANDROID)/Include/RepXUpgrader",
				"$(PHYSX_SDK_ANDROID)/Include/vehicle",
			}

		configuration { "vs*" }
			includedirs {
				"$(PHYSX_SDK_WINDOWS)/Include",
				"$(PHYSX_SDK_WINDOWS)/Include/common",
				"$(PHYSX_SDK_WINDOWS)/Include/characterkinematic",
				"$(PHYSX_SDK_WINDOWS)/Include/cloth",
				"$(PHYSX_SDK_WINDOWS)/Include/common",
				"$(PHYSX_SDK_WINDOWS)/Include/cooking",
				"$(PHYSX_SDK_WINDOWS)/Include/extensions",
				"$(PHYSX_SDK_WINDOWS)/Include/foundation",
				"$(PHYSX_SDK_WINDOWS)/Include/geometry",
				"$(PHYSX_SDK_WINDOWS)/Include/particles",
				"$(PHYSX_SDK_WINDOWS)/Include/physxprofilesdk",
				"$(PHYSX_SDK_WINDOWS)/Include/physxvisualdebuggersdk",
				"$(PHYSX_SDK_WINDOWS)/Include/pvd",
				"$(PHYSX_SDK_WINDOWS)/Include/pxtask",
				"$(PHYSX_SDK_WINDOWS)/Include/RepX",
				"$(PHYSX_SDK_WINDOWS)/Include/RepXUpgrader",
				"$(PHYSX_SDK_WINDOWS)/Include/vehicle",
				"$(DXSDK_DIR)/Include",
			}

		configuration { "linux-*" }
			links {
				"X11",
				"Xrandr",
				"pthread",
				"GL",
				"dl",
			}

		configuration { "debug", "linux-*" }
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
				"OpenSLES",
			}

		configuration { "debug", "android-arm" }
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
				"dbghelp",
			}

		configuration { "debug", "x32", "vs*"}
			links {
				"PhysX3CharacterKinematicCHECKED_x86",
				"PhysX3CHECKED_x86",
				"PhysX3CommonCHECKED_x86",
				"PhysX3CookingCHECKED_x86",
				"PhysX3ExtensionsCHECKED",
			}

		configuration { "debug", "x64", "vs*" }
			links {
				"PhysX3CharacterKinematicCHECKED_x64",
				"PhysX3CHECKED_x64",
				"PhysX3CommonCHECKED_x64",
				"PhysX3CookingCHECKED_x64",
				"PhysX3ExtensionsCHECKED",
			}

		configuration { "development", "x32", "vs*" }
			links {
				"PhysX3CharacterKinematicPROFILE_x86",
				"PhysX3PROFILE_x86",
				"PhysX3CommonPROFILE_x86",
				"PhysX3CookingPROFILE_x86",
				"PhysX3ExtensionsPROFILE",
			}

		configuration { "development", "x64", "vs*" }
			links {
				"PhysX3CharacterKinematicPROFILE_x64",
				"PhysX3PROFILE_x64",
				"PhysX3CommonPROFILE_x64",
				"PhysX3CookingPROFILE_x64",
				"PhysX3ExtensionsPROFILE",
			}

		configuration { "release", "x32", "vs*" }
			links {
				"PhysX3CharacterKinematic_x86",
				"PhysX3_x86",
				"PhysX3Common_x86",
				"PhysX3Cooking_x86",
				"PhysX3Extensions",
			}

		configuration { "release", "x64", "vs*" }
			links {
				"PhysX3CharacterKinematic_x64",
				"PhysX3_x64",
				"PhysX3Common_x64",
				"PhysX3Cooking_x64",
				"PhysX3Extensions",
			}

		configuration {}

		files {
			CROWN_DIR .. "engine/**.h",
			CROWN_DIR .. "engine/**.cpp"
		}

		strip()

		configuration {} -- reset configuration
end
