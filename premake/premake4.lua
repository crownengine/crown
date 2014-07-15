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

-------------------------------------------------------------------------------
-- Globals

CROWN_SOURCE_DIR = path.getabsolute("..") .. "/"
CROWN_THIRD_DIR = CROWN_SOURCE_DIR .. "third/"
CROWN_BUILD_DIR = "../.build/"
CROWN_INSTALL_DIR = "../.installation/"

-------------------------------------------------------------------------------
-- Options

newoption
{
	trigger = "compiler",
	value = "COMPILER",
	description = "Choose compiler",
	allowed =
	{
		{ "android-arm", "Android - ARM" },			-- gcc
		-- { "android-mips", "Android - MIPS" },			-- gcc
		-- { "android-x86", "Android - x86" },				-- gcc
		{ "linux-gcc", "Linux (GCC compiler)" },		-- gcc
		-- { "linux-clang", "Linux (Clang compiler)" },		-- clang
		-- { "osx-gcc", "OSX" },							-- gcc
		-- { "ios-arm", "iOS - ARM" },						-- clang
		-- { "ios-simulator", "iOS - Simulator" }			-- clang
	}
}

-- Avoid error invoking premake4 --help
if _ACTION == nil then return end

if _ACTION == "clean" then os.rmdir(CROWN_BUILD_DIR) end

if _ACTION == "gmake" then

	if _OPTIONS["compiler"] == "linux-gcc" then
		-- dunno
	elseif _OPTIONS["compiler"] == "android-arm" then
		if not os.getenv("ANDROID_NDK_ARM") or not os.getenv("ANDROID_NDK_ROOT") then
			print("Set ANDROID_NDK_ARM and ANDROID_NDK_ROOT environment variables.")
		end

		premake.gcc.cc = "$(ANDROID_NDK_ARM)/bin/arm-linux-androideabi-gcc"
		premake.gcc.cxx = "$(ANDROID_NDK_ARM)/bin/arm-linux-androideabi-g++"
		premake.gcc.ar = "$(ANDROID_NDK_ARM)/bin/arm-linux-androideabi-ar"
	end

end

-------------------------------------------------------------------------------
-- Solution
solution "crown"
	configurations { "debug", "development", "release" }
	platforms { "x32", "x64" }

	configuration { "debug" }
		targetsuffix "-debug"
		defines { "_DEBUG", "CROWN_DEBUG" }
	configuration { "development" }
		flags { "OptimizeSpeed" }
		targetsuffix "-development"
		defines { "NDEBUG", "CROWN_DEVELOPMENT" }
	configuration { "release" }
		flags { "OptimizeSpeed" }
		targetsuffix "-release"
		defines { "NDEBUG", "CROWN_RELEASE" }

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

		configuration { "linux" }
			kind "ConsoleApp"
			location(".build/linux") -- its ok
			targetdir(".installation/linux") -- must be specified by user -- tmp

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

			includedirs
			{
				CROWN_SOURCE_DIR .. "/engine/os/linux",
				CROWN_SOURCE_DIR .. "/engine/renderers/backend/gl/glx"
			}

			files
			{
				CROWN_SOURCE_DIR .. "engine/**.h", 
				CROWN_SOURCE_DIR .. "engine/**.cpp"
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

