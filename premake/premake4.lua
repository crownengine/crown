-- Copyright (c) 2013 - 2014 Daniele Bartolini, Michele Rossi
-- Copyright (c) 2012 Daniele Bartolini, Simone Boscaratto
-- 
-- Permission is hereby granted, free of charge, to any person
-- obtaining a copy of this software and associated documentation
-- files (the "Software"), to deal in the Software without
-- restriction, including without limitation the rights to use,
-- copy, modify, merge, publish, distribute, sublicense, and/or sell
-- copies of the Software, and to permit persons to whom the
-- Software is furnished to do so, subject to the following
-- conditions:
-- 
-- The above copyright notice and this permission notice shall be
-- included in all copies or substantial portions of the Software.
-- 
-- THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
-- EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
-- OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
-- NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
-- HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
-- WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
-- FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
-- OTHER DEALINGS IN THE SOFTWARE.

------------------------------------------------------------------------------
-- Options

newoption
{
	trigger = "install-dir",
	value = "DIR",
	description = "Output directory"
}

newoption
{
	trigger = "compiler",
	value = "COMPILER",
	description = "Choose compiler",
	allowed =
	{
		{ "android", "Android (ARM only)" },			-- gcc
		-- { "android-mips", "Android - MIPS" },			-- gcc
		-- { "android-x86", "Android - x86" },				-- gcc
		{ "linux-gcc", "Linux (GCC compiler)" },		-- gcc
		-- { "linux-clang", "Linux (Clang compiler)" },		-- clang
		-- { "osx-gcc", "OSX" },							-- gcc
		-- { "ios-arm", "iOS - ARM" },						-- clang
		-- { "ios-simulator", "iOS - Simulator" }			-- clang
	}
}

-- if _OPTIONS["install-dir"] == nil then 
-- 	print("'install-dir' option must be specified")
-- 	os.exit(1)
-- end

-- if not path.isabsolute(_OPTIONS["install-dir"]) then
-- 	print("'install-dir' must be an absolute path")
-- 	os.exit(1)
-- end

CROWN_SOURCE_DIR = path.getabsolute("..") .. "/"
CROWN_THIRD_DIR = CROWN_SOURCE_DIR .. "third/"
CROWN_BUILD_DIR = "../.build/"
CROWN_INSTALL_DIR = _OPTIONS["install-dir"]

-- Avoid error invoking premake4 --help
if _ACTION == nil then return end

if _ACTION == "clean" then os.rmdir(CROWN_BUILD_DIR) end

if _ACTION == "gmake" then

	if _OPTIONS["compiler"] == "linux-gcc" then
		-- dunno
	elseif _OPTIONS["compiler"] == "android" then
		if not os.getenv("ANDROID_NDK") then
			print("Set ANDROID_NDK environment variable.")
		end

		premake.gcc.cc = "$(ANDROID_NDK)/toolchains/arm-linux-androideabi-4.8/prebuilt/linux-x86_64/bin/arm-linux-androideabi-gcc"
		premake.gcc.cxx = "$(ANDROID_NDK)/toolchains/arm-linux-androideabi-4.8/prebuilt/linux-x86_64/bin/arm-linux-androideabi-g++"
		premake.gcc.ar = "$(ANDROID_NDK)/toolchains/arm-linux-androideabi-4.8/prebuilt/linux-x86_64/bin/arm-linux-androideabi-ar"
	end
end

-------------------------------------------------------------------------------
-- Solution
solution "crown"
	configurations { "debug", "development", "release" }
	platforms { "Native", "x32", "x64" }

	configuration { "debug" }
		defines { "_DEBUG", "CROWN_DEBUG" }
	configuration { "development" }
		flags { "OptimizeSpeed" }
		defines { "NDEBUG", "CROWN_DEVELOPMENT" }
	configuration { "release" }
		flags { "OptimizeSpeed" }
		defines { "NDEBUG", "CROWN_RELEASE" }

	configuration { "debug", "x32" }
		targetsuffix "-debug-32"
	configuration { "debug", "x64" }
		targetsuffix "-debug-64"

	configuration { "development", "x32" }
		targetsuffix "-development-32"
	configuration { "development", "x64" }
		targetsuffix "-development-64"

	configuration { "release", "x32" }
		targetsuffix "-release-32"
	configuration { "release", "x64" }
		targetsuffix "-release-64"


	flags
	{
		"StaticRuntime",
		"NoMinimalRebuild",
		"NoPCH",
		"NativeWChar",
		"NoRTTI",
		"NoExceptions",
		"NoEditAndContinue",
		"Symbols"
	}

	-------------------------------------------------------------------------------
	project "crown"
		language "C++"

		includedirs
		{
			CROWN_SOURCE_DIR .. "/engine",
			CROWN_SOURCE_DIR .. "/engine/core",
			CROWN_SOURCE_DIR .. "/engine/core/bv",
			CROWN_SOURCE_DIR .. "/engine/core/containers",
			CROWN_SOURCE_DIR .. "/engine/core/math",
			CROWN_SOURCE_DIR .. "/engine/core/mem",
			CROWN_SOURCE_DIR .. "/engine/core/compressors",
			CROWN_SOURCE_DIR .. "/engine/core/filesystem",
			CROWN_SOURCE_DIR .. "/engine/core/json",
			CROWN_SOURCE_DIR .. "/engine/core/strings",
			CROWN_SOURCE_DIR .. "/engine/core/settings",
			CROWN_SOURCE_DIR .. "/engine/os",
			CROWN_SOURCE_DIR .. "/engine/input",
			CROWN_SOURCE_DIR .. "/engine/renderers",
			CROWN_SOURCE_DIR .. "/engine/renderers/backend",
			CROWN_SOURCE_DIR .. "/engine/resource",
			CROWN_SOURCE_DIR .. "/engine/rpc",
			CROWN_SOURCE_DIR .. "/engine/network",
			CROWN_SOURCE_DIR .. "/engine/lua",
			CROWN_SOURCE_DIR .. "/engine/audio",
			CROWN_SOURCE_DIR .. "/engine/compilers",
			CROWN_SOURCE_DIR .. "/engine/physics",
			CROWN_SOURCE_DIR .. "/engine/world"
		}

		files
		{
			CROWN_SOURCE_DIR .. "engine/**.h", 
			CROWN_SOURCE_DIR .. "engine/**.cpp"
		}

		configuration { "linux" }
			kind "ConsoleApp"
			location(CROWN_BUILD_DIR .. "linux") -- its ok
			targetdir("../.installation/linux") -- must be specified by user -- tmp

			defines { "CROWN_LINUX" }

			buildoptions
			{
				"-std=c++03",	
				"-O0"
			}
			
			linkoptions
			{
				"-Wl,-rpath=\\$$ORIGIN"
			}

			links
			{
				"Xrandr",
				"pthread",
				"dl",
				"GL",
				"X11",
				"openal",
				"luajit-5.1"
			}

			includedirs
			{
				CROWN_SOURCE_DIR .. "/engine/os/linux",
				CROWN_SOURCE_DIR .. "/engine/renderers/backend/gl/glx"
			}

			excludes
			{
				CROWN_SOURCE_DIR .. "engine/os/android/*",
				CROWN_SOURCE_DIR .. "engine/os/win/*",
				CROWN_SOURCE_DIR .. "engine/renderers/backend/gl/egl/*",
				CROWN_SOURCE_DIR .. "engine/renderers/backend/gl/wgl/*",
				CROWN_SOURCE_DIR .. "engine/audio/backend/SLESSoundWorld.cpp"
			}

		configuration { "debug", "linux" }
			linkoptions
			{ 
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
		configuration { "development", "linux" }
			linkoptions
			{ 
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
		configuration { "release", "linux" }
			linkoptions
			{ 
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

		configuration { "linux", "x32" }
			buildoptions
			{
				"-m32"
			}
			includedirs {
				CROWN_THIRD_DIR .. "luajit/x86/include/luajit-2.0",
				CROWN_THIRD_DIR .. "physx/x86/include",
				CROWN_THIRD_DIR .. "physx/x86/include/common",
				CROWN_THIRD_DIR .. "physx/x86/include/characterkinematic",
				CROWN_THIRD_DIR .. "physx/x86/include/cloth",
				CROWN_THIRD_DIR .. "physx/x86/include/common",
				CROWN_THIRD_DIR .. "physx/x86/include/cooking",
				CROWN_THIRD_DIR .. "physx/x86/include/extensions",
				CROWN_THIRD_DIR .. "physx/x86/include/foundation",
				CROWN_THIRD_DIR .. "physx/x86/include/geometry",
				CROWN_THIRD_DIR .. "physx/x86/include/particles",
				CROWN_THIRD_DIR .. "physx/x86/include/physxprofilesdk",
				CROWN_THIRD_DIR .. "physx/x86/include/physxvisualdebuggersdk",
				CROWN_THIRD_DIR .. "physx/x86/include/pvd",
				CROWN_THIRD_DIR .. "physx/x86/include/pxtask",
				CROWN_THIRD_DIR .. "physx/x86/include/RepX",
				CROWN_THIRD_DIR .. "physx/x86/include/RepXUpgrader",
				CROWN_THIRD_DIR .. "physx/x86/include/vehicle",
				CROWN_THIRD_DIR .. "opengl",
				CROWN_THIRD_DIR .. "openal/include",
				CROWN_THIRD_DIR .. "freetype",
				CROWN_THIRD_DIR .. "stb_image",
				CROWN_THIRD_DIR .. "stb_vorbis"
			}

			libdirs
			{
				CROWN_THIRD_DIR .. "luajit/x86/lib",
				CROWN_THIRD_DIR .. "physx/x86/lib"
			}

		configuration { "linux", "x64" }
			buildoptions
			{
				"-m64"
			}
			includedirs {
				CROWN_THIRD_DIR .. "luajit/x86_64/include/luajit-2.0",
				CROWN_THIRD_DIR .. "physx/x86_64/include",
				CROWN_THIRD_DIR .. "physx/x86_64/include/common",
				CROWN_THIRD_DIR .. "physx/x86_64/include/characterkinematic",
				CROWN_THIRD_DIR .. "physx/x86_64/include/cloth",
				CROWN_THIRD_DIR .. "physx/x86_64/include/common",
				CROWN_THIRD_DIR .. "physx/x86_64/include/cooking",
				CROWN_THIRD_DIR .. "physx/x86_64/include/extensions",
				CROWN_THIRD_DIR .. "physx/x86_64/include/foundation",
				CROWN_THIRD_DIR .. "physx/x86_64/include/geometry",
				CROWN_THIRD_DIR .. "physx/x86_64/include/particles",
				CROWN_THIRD_DIR .. "physx/x86_64/include/physxprofilesdk",
				CROWN_THIRD_DIR .. "physx/x86_64/include/physxvisualdebuggersdk",
				CROWN_THIRD_DIR .. "physx/x86_64/include/pvd",
				CROWN_THIRD_DIR .. "physx/x86_64/include/pxtask",
				CROWN_THIRD_DIR .. "physx/x86_64/include/RepX",
				CROWN_THIRD_DIR .. "physx/x86_64/include/RepXUpgrader",
				CROWN_THIRD_DIR .. "physx/x86_64/include/vehicle",
				CROWN_THIRD_DIR .. "opengl",
				CROWN_THIRD_DIR .. "openal/include",
				CROWN_THIRD_DIR .. "freetype",
				CROWN_THIRD_DIR .. "stb_image",
				CROWN_THIRD_DIR .. "stb_vorbis"
			}

			libdirs
			{
				CROWN_THIRD_DIR .. "luajit/x86_64/lib",
				CROWN_THIRD_DIR .. "physx/x86_64/lib"
			}

		-- configuration { "android" }
		-- 	kind "SharedLib"
		-- 	location(".build/android") -- its ok
		-- 	targetdir(".installation/android") -- must be specified by user -- tmp

		-- 	defines { "CROWN_ANDROID" }

		-- 	buildoptions
		-- 	{
		-- 		"-std=c++03"
		-- 	}
			
		-- 	linkoptions
		-- 	{
		-- 		"-Wl,-rpath=\\$$ORIGIN",
		-- 		-- "-llog -landroid -lEGL -lGLESv2 -lz -lOpenSLES"
		-- 		"-Wl,--start-group $(addprefix -l," ..
		-- 		"	LowLevelClothCHECKED" ..
		-- 		"	PhysX3CHECKED " ..
		-- 		"	PhysX3CommonCHECKED" ..
		-- 		"	PxTaskCHECKED" ..
		-- 		"	LowLevelCHECKED" ..
		-- 		"	PhysX3CharacterKinematicCHECKED" ..
		-- 		"	PhysX3CookingCHECKED" ..
		-- 		"	PhysX3ExtensionsCHECKED" ..
		-- 		"	PhysX3VehicleCHECKED" ..
		-- 		"	PhysXProfileSDKCHECKED" ..
		-- 		"	PhysXVisualDebuggerSDKCHECKED" ..
		-- 		"	PvdRuntimeCHECKED" ..
		-- 		"	SceneQueryCHECKED" ..
		-- 		"	SimulationControllerCHECKED" ..
		-- 		") -Wl,--end-group"
		-- 	}

		-- 	links
		-- 	{
		-- 		"log",
		-- 		"android",
		-- 		"EGL",
		-- 		"GLESv2",
		-- 		"z",
		-- 		"OpenSLES",
		-- 	}

		-- 	includedirs {
		-- 		CROWN_SOURCE_DIR .. "engine/os/android",
		-- 		CROWN_THIRD_DIR .. "luajit/android/include/luajit-2.0",
		-- 		CROWN_THIRD_DIR .. "physx/android/include",
		-- 		CROWN_THIRD_DIR .. "physx/android/include/common",
		-- 		CROWN_THIRD_DIR .. "physx/android/include/characterkinematic",
		-- 		CROWN_THIRD_DIR .. "physx/android/include/cloth",
		-- 		CROWN_THIRD_DIR .. "physx/android/include/common",
		-- 		CROWN_THIRD_DIR .. "physx/android/include/cooking",
		-- 		CROWN_THIRD_DIR .. "physx/android/include/extensions",
		-- 		CROWN_THIRD_DIR .. "physx/android/include/foundation",
		-- 		CROWN_THIRD_DIR .. "physx/android/include/geometry",
		-- 		CROWN_THIRD_DIR .. "physx/android/include/particles",
		-- 		CROWN_THIRD_DIR .. "physx/android/include/physxprofilesdk",
		-- 		CROWN_THIRD_DIR .. "physx/android/include/physxvisualdebuggersdk",
		-- 		CROWN_THIRD_DIR .. "physx/android/include/pvd",
		-- 		CROWN_THIRD_DIR .. "physx/android/include/pxtask",
		-- 		CROWN_THIRD_DIR .. "physx/android/include/RepX",
		-- 		CROWN_THIRD_DIR .. "physx/android/include/RepXUpgrader",
		-- 		CROWN_THIRD_DIR .. "physx/android/include/vehicle",
		-- 		CROWN_THIRD_DIR .. "opengl",
		-- 		CROWN_THIRD_DIR .. "openal/include",
		-- 		CROWN_THIRD_DIR .. "freetype",
		-- 		CROWN_THIRD_DIR .. "stb_image",
		-- 		CROWN_THIRD_DIR .. "stb_vorbis",
		-- 		"$(ANDROID_NDK)/sources/cxx-stl/gnu-libstdc++/4.8/include",
		-- 		-- "$(ANDROID_NDK)/sources/cxx-stl/gnu-libstdc++/4.8/libs/armeabi-v7a/include",
		-- 		"$(ANDROID_NDK)/sources/android/native_app_glue",
		-- 	}

		-- 	libdirs
		-- 	{
		-- 		CROWN_THIRD_DIR .. "luajit/android/lib",
		-- 		CROWN_THIRD_DIR .. "physx/android/lib",
		-- 		"$(ANDROID_NDK)/sources/cxx-stl/gnu-libstdc++/4.8/libs/armeabi-v7a",
		-- 		"$(ANDROID_NDK)/platforms/android-18/arch-arm/usr/lib"
		-- 	}

		-- 	excludes
		-- 	{
		-- 		CROWN_SOURCE_DIR .. "engine/os/linux/*",
		-- 		CROWN_SOURCE_DIR .. "engine/os/win/*",
		-- 		CROWN_SOURCE_DIR .. "engine/renderers/backend/gl/glx/*",
		-- 		CROWN_SOURCE_DIR .. "engine/renderers/backend/gl/wgl/*",
		-- 		CROWN_SOURCE_DIR .. "engine/audio/backend/ALSoundWorld.cpp"
		-- 	}

		-- configuration { "debug", "android" }
		-- 	linkoptions
		-- 	{
		-- 		"-Wl,--start-group $(addprefix -l," ..
		-- 		"	LowLevelClothCHECKED" ..
		-- 		"	PhysX3CHECKED " ..
		-- 		"	PhysX3CommonCHECKED" ..
		-- 		"	PxTaskCHECKED" ..
		-- 		"	LowLevelCHECKED" ..
		-- 		"	PhysX3CharacterKinematicCHECKED" ..
		-- 		"	PhysX3CookingCHECKED" ..
		-- 		"	PhysX3ExtensionsCHECKED" ..
		-- 		"	PhysX3VehicleCHECKED" ..
		-- 		"	PhysXProfileSDKCHECKED" ..
		-- 		"	PhysXVisualDebuggerSDKCHECKED" ..
		-- 		"	PvdRuntimeCHECKED" ..
		-- 		"	SceneQueryCHECKED" ..
		-- 		"	SimulationControllerCHECKED" ..
		-- 		") -Wl,--end-group"
		-- 	}
		-- configuration { "development", "android"}
		-- 	linkoptions
		-- 	{ 
		-- 		"-Wl,--start-group $(addprefix -l," ..
		-- 		"	LowLevelClothPROFILE" ..
		-- 		"	PhysX3PROFILE " ..
		-- 		"	PhysX3CommonPROFILE" ..
		-- 		"	PxTaskPROFILE" ..
		-- 		"	LowLevelPROFILE" ..
		-- 		"	PhysX3CharacterKinematicPROFILE" ..
		-- 		"	PhysX3CookingPROFILE" ..
		-- 		"	PhysX3ExtensionsPROFILE" ..
		-- 		"	PhysX3VehiclePROFILE" ..
		-- 		"	PhysXProfileSDKPROFILE" ..
		-- 		"	PhysXVisualDebuggerSDKPROFILE" ..
		-- 		"	PvdRuntimePROFILE" ..
		-- 		"	SceneQueryPROFILE" ..
		-- 		"	SimulationControllerPROFILE" ..
		-- 		") -Wl,--end-group"
		-- 	}		
		-- configuration { "release", "android"}
		-- 	linkoptions
		-- 	{ 
		-- 		"-Wl,--start-group $(addprefix -l," ..
		-- 		"	LowLevelCloth" ..
		-- 		"	PhysX3 " ..
		-- 		"	PhysX3Common" ..
		-- 		"	PxTask" ..
		-- 		"	LowLevel" ..
		-- 		"	PhysX3CharacterKinematic" ..
		-- 		"	PhysX3Cooking" ..
		-- 		"	PhysX3Extensions" ..
		-- 		"	PhysX3Vehicle" ..
		-- 		"	PhysXProfileSDK" ..
		-- 		"	PhysXVisualDebuggerSDK" ..
		-- 		"	PvdRuntime" ..
		-- 		"	SceneQuery" ..
		-- 		"	SimulationController" ..
		-- 		") -Wl,--end-group"
		-- 	}