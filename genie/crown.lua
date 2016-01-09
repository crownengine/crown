--
-- Copyright (c) 2012-2016 Daniele Bartolini and individual contributors.
-- License: https://github.com/taylor001/crown/blob/master/LICENSE
--

function crown_project(_name, _kind, _defines)

	project ("crown" .. _name)
		kind (_kind)

		includedirs {
			CROWN_DIR .. "src",
			CROWN_DIR .. "src/audio",
			CROWN_DIR .. "src/compilers",
			CROWN_DIR .. "src/core",
			CROWN_DIR .. "src/core/containers",
			CROWN_DIR .. "src/core/error",
			CROWN_DIR .. "src/core/filesystem",
			CROWN_DIR .. "src/core/json",
			CROWN_DIR .. "src/core/math",
			CROWN_DIR .. "src/core/memory",
			CROWN_DIR .. "src/core/network",
			CROWN_DIR .. "src/core/strings",
			CROWN_DIR .. "src/core/thread",
			CROWN_DIR .. "src/input",
			CROWN_DIR .. "src/lua",
			CROWN_DIR .. "src/main",
			CROWN_DIR .. "src/physics",
			CROWN_DIR .. "src/renderers",
			CROWN_DIR .. "src/resource",
			CROWN_DIR .. "src/world",
			CROWN_DIR .. "third/bx/include",
			CROWN_DIR .. "third/bgfx/include",
			CROWN_DIR .. "third/bgfx/src",
			CROWN_DIR .. "third/stb_vorbis",
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

			links { "openal" }

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

		configuration { "linux-*" }
			links {
				"X11",
				"Xrandr",
				"pthread",
				"dl",
				"GL",
			}

		configuration { "vs*" }
			links {
				"OpenGL32",
				"dbghelp",
				"xinput",
			}

		-- PhysX
		local function includedirs_physx(prefix)
			includedirs {
				prefix .. "Include",
				prefix .. "Include/common",
				prefix .. "Include/characterkinematic",
				prefix .. "Include/cloth",
				prefix .. "Include/common",
				prefix .. "Include/cooking",
				prefix .. "Include/extensions",
				prefix .. "Include/foundation",
				prefix .. "Include/geometry",
				prefix .. "Include/particles",
				prefix .. "Include/physxprofilesdk",
				prefix .. "Include/physxvisualdebuggersdk",
				prefix .. "Include/pvd",
				prefix .. "Include/pxtask",
				prefix .. "Include/RepX",
				prefix .. "Include/RepXUpgrader",
				prefix .. "Include/vehicle",
			}
		end

		local function links_physx(config, os, platform)
			if os == "linux" then
				linkoptions {
					"-Wl,--start-group $(addprefix -l," ..
					"	PhysX3"                   .. config .. "_" .. platform ..
					"	PhysX3Common"             .. config .. "_" .. platform ..
					"	PhysX3Cooking"            .. config .. "_" .. platform ..
					"	PhysX3CharacterKinematic" .. config .. "_" .. platform ..
					"	PhysX3Extensions"         .. config ..
					") -Wl,--end-group"
				}
			end

			if os == "android" then
				linkoptions {
					"-Wl,--start-group $(addprefix -l," ..
					"	LowLevelCloth"            .. config ..
					"	PhysX3 "                  .. config ..
					"	PhysX3Common"             .. config ..
					"	PxTask"                   .. config ..
					"	LowLevel"                 .. config ..
					"	PhysX3CharacterKinematic" .. config ..
					"	PhysX3Cooking"            .. config ..
					"	PhysX3Extensions"         .. config ..
					"	PhysX3Vehicle"            .. config ..
					"	PhysXProfileSDK"          .. config ..
					"	PhysXVisualDebuggerSDK"   .. config ..
					"	PvdRuntime"               .. config ..
					"	SceneQuery"               .. config ..
					"	SimulationController"     .. config ..
					") -Wl,--end-group"
				}
			end

			if os == "windows" then
				links {
					"PhysX3"                   .. config .. "_" .. platform,
					"PhysX3Common"             .. config .. "_" .. platform,
					"PhysX3Cooking"            .. config .. "_" .. platform,
					"PhysX3CharacterKinematic" .. config .. "_" .. platform,
					"PhysX3Extensions",
				}
			end
		end

		configuration { "android*" }
			includedirs_physx("$(PHYSX_SDK_ANDROID)/")

		configuration { "linux*" }
			includedirs_physx("$(PHYSX_SDK_LINUX)/")

		configuration { "vs*" }
			includedirs_physx("$(PHYSX_SDK_WINDOWS)/")

		configuration { "android-arm" }
			libdirs {
				"$(PHYSX_SDK_ANDROID)/Lib/android9_neon",
			}

		configuration { "x32", "linux-*" }
			libdirs {
				"$(PHYSX_SDK_LINUX)/Lib/linux32",
				"$(PHYSX_SDK_LINUX)/Bin/linux32",
			}

		configuration { "x64", "linux-*" }
			libdirs {
				"$(PHYSX_SDK_LINUX)/Lib/linux64",
				"$(PHYSX_SDK_LINUX)/Bin/linux64",
			}

		configuration { "x32", "vs*" }
			libdirs {
				"$(PHYSX_SDK_WINDOWS)/Lib/win32",
			}

		configuration { "x64", "vs*" }
			libdirs {
				"$(PHYSX_SDK_WINDOWS)/Lib/win64",
			}

		configuration { "x32", "debug", "linux-*" }
			links_physx("DEBUG", "linux", "x86")

		configuration { "x64", "debug", "linux-*" }
			links_physx("DEBUG", "linux", "x64")

		configuration { "x32", "development", "linux-*" }
			links_physx("CHECKED", "linux", "x86")

		configuration { "x64", "development", "linux-*" }
			links_physx("CHECKED", "linux", "x64")

		configuration { "x32", "release", "linux-*" }
			links_physx("", "linux", "x86")

		configuration { "x64", "release", "linux-*" }
			links_physx("", "linux", "x64")

		configuration { "debug", "android-arm" }
			links_physx("DEBUG", "android", "")

		configuration { "development", "android-arm" }
			links_physx("CHECKED", "android", "")

		configuration { "release", "android-arm" }
			links_physx("", "android", "")

		configuration { "debug", "x32", "vs*"}
			links_physx("DEBUG", "windows", "x86")

		configuration { "debug", "x64", "vs*" }
			links_physx("DEBUG", "windows", "x64")

		configuration { "development", "x32", "vs*" }
			links_physx("CHECKED", "windows", "x86")

		configuration { "development", "x64", "vs*" }
			links_physx("CHECKED", "windows", "x64")

		configuration { "release", "x32", "vs*" }
			links_physx("", "windows", "x86")

		configuration { "release", "x64", "vs*" }
			links_physx("", "windows", "x64")

		configuration {}

		files {
			CROWN_DIR .. "src/**.h",
			CROWN_DIR .. "src/**.cpp"
		}

		strip()

		configuration {} -- reset configuration
end
