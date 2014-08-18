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
		{ "linux-gcc", "Linux (GCC compiler)" },		-- gcc
	}
}

CROWN_SOURCE_DIR = path.getabsolute("..") .. "/"
CROWN_THIRD_DIR = CROWN_SOURCE_DIR .. "third/"
CROWN_BUILD_DIR = CROWN_SOURCE_DIR .. ".build/"
CROWN_INSTALL_DIR = os.getenv("CROWN_INSTALL_DIR")
if not CROWN_INSTALL_DIR then
	if not path.isabsolute(CROWN_INSTALL_DIR) then
		CROWN_INSTALL_DIR = CROWN_SOURCE_DIR .. ".install"
	end
end
CROWN_INSTALL_DIR = CROWN_INSTALL_DIR .. "/" -- Add slash to end string

-------------------------------------------------------------------------------
-- Solution
solution "crown"
	configurations { "debug", "development", "release" }
	platforms { "x32", "x64", "native" }

	-- Avoid error invoking premake4 --help
	if _ACTION == nil then return end

	if _ACTION == "clean" then os.rmdir(CROWN_BUILD_DIR) end

	if _ACTION == "gmake" then

		if _OPTIONS["compiler"] == "linux-gcc" then

			if not os.is("linux") then print("Action not valid in current OS.") end

			if not os.getenv("PHYSX_SDK_LINUX") then
				print("Set PHYSX_SDK_LINUX environment variable.")
			end

			location(CROWN_BUILD_DIR .. "linux")

		elseif _OPTIONS["compiler"] == "android" then

			if not os.getenv("ANDROID_NDK_ROOT") then
				print("Set ANDROID_NDK_ROOT environment variable.")
			end
			if not os.getenv("ANDROID_NDK_ARM") then
				print("Set ANDROID_NDK_ARM environment variables.")
			end
			if not os.getenv("PHYSX_SDK_ANDROID") then
				print("Set PHYSX_SDK_ANDROID environment variable.")
			end

			premake.gcc.cc = "$(ANDROID_NDK_ARM)/bin/arm-linux-androideabi-gcc"
			premake.gcc.cxx = "$(ANDROID_NDK_ARM)/bin/arm-linux-androideabi-g++"
			premake.gcc.ar = "$(ANDROID_NDK_ARM)/bin/arm-linux-androideabi-ar"

			location(CROWN_BUILD_DIR .. "android")

		end
	end

	if _ACTION == "vs2010" or _ACTION == "vs2008" then

		if not os.is("windows") then print("Action not valid in current OS.") end

		if not os.getenv("PHYSX_SDK_WINDOWS") then
			print("Set PHYSX_SDK_WINDOWS environment variable.")
		end

		location(CROWN_BUILD_DIR .. "windows")

	end

	flags {
		"NoMinimalRebuild",
		"NoPCH",
		"NoRTTI",
		"NoExceptions",
		"NoEditAndContinue",
	}


	configuration { "debug" }
		flags { "Symbols" }
		defines { "_DEBUG", "CROWN_DEBUG" }
	configuration { "development" }
		flags { "Symbols" }
		defines { "_DEBUG", "CROWN_DEBUG" }
	configuration { "release" }
		defines { "NDEBUG" }

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

	configuration { "debug", "native" }
		targetsuffix "-debug"

	configuration { "development", "native" }
		targetsuffix "-development"

	configuration { "release", "native" }
		targetsuffix "-release"

	-------------------------------------------------------------------------------
	project "crown"
		language "C++"

		includedirs {
			CROWN_SOURCE_DIR .. "/engine",
			CROWN_SOURCE_DIR .. "/engine/core",
			CROWN_SOURCE_DIR .. "/engine/core/bv",
			CROWN_SOURCE_DIR .. "/engine/core/containers",
			CROWN_SOURCE_DIR .. "/engine/core/filesystem",
			CROWN_SOURCE_DIR .. "/engine/core/json",
			CROWN_SOURCE_DIR .. "/engine/core/math",
			CROWN_SOURCE_DIR .. "/engine/core/mem",
			CROWN_SOURCE_DIR .. "/engine/core/network",
			CROWN_SOURCE_DIR .. "/engine/core/settings",
			CROWN_SOURCE_DIR .. "/engine/core/strings",
			CROWN_SOURCE_DIR .. "/engine/core/thread",
			CROWN_SOURCE_DIR .. "/engine/os",
			CROWN_SOURCE_DIR .. "/engine/input",
			CROWN_SOURCE_DIR .. "/engine/renderers",
			CROWN_SOURCE_DIR .. "/engine/renderers/backend",
			CROWN_SOURCE_DIR .. "/engine/resource",
			CROWN_SOURCE_DIR .. "/engine/lua",
			CROWN_SOURCE_DIR .. "/engine/audio",
			CROWN_SOURCE_DIR .. "/engine/compilers",
			CROWN_SOURCE_DIR .. "/engine/physics",
			CROWN_SOURCE_DIR .. "/engine/world"
		}

		files {
			CROWN_SOURCE_DIR .. "engine/**.h", 
			CROWN_SOURCE_DIR .. "engine/**.cpp"
		}

		configuration { "linux-*" }
			kind "ConsoleApp"

			buildoptions {
				"-std=c++03",
				"-Wall",
				-- "-Wextra",
				-- "-Werror",
				-- "-pedantic",
				"-Wno-unknown-pragmas",
				"-Wno-unused-local-typedefs"
			}
			
			linkoptions {
				"-Wl,-rpath=\\$$ORIGIN"
			}

			links {
				"Xrandr",
				"pthread",
				"GL",
				"X11",
				"openal",
				"luajit",
				"dl",
			}

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
				CROWN_SOURCE_DIR .. "/engine/os/linux",
				CROWN_SOURCE_DIR .. "/engine/renderers/backend/gl/glx",
				CROWN_THIRD_DIR .. "luajit/src",
				CROWN_THIRD_DIR .. "opengl",
				CROWN_THIRD_DIR .. "openal/include",
				CROWN_THIRD_DIR .. "freetype",
				CROWN_THIRD_DIR .. "stb_image",
				CROWN_THIRD_DIR .. "stb_vorbis",
			}

			excludes {
				CROWN_SOURCE_DIR .. "engine/os/android/*",
				CROWN_SOURCE_DIR .. "engine/os/win/*",
				CROWN_SOURCE_DIR .. "engine/renderers/backend/gl/egl/*",
				CROWN_SOURCE_DIR .. "engine/renderers/backend/gl/wgl/*",
				CROWN_SOURCE_DIR .. "engine/audio/backend/sles_sound_world.cpp",
			}
			
		configuration { "linux-*", "debug" }
			buildoptions {
				"-O0"
			}

			linkoptions { 
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

		configuration { "linux-*", "development" }
			buildoptions {
				"-O2"
			}

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

		configuration { "linux-*", "release" }
			buildoptions {
				"-O2"
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

		configuration { "linux-*", "x32" }
			targetdir(CROWN_INSTALL_DIR .. "bin/linux32")
		
			buildoptions {
				"-malign-double" -- Required by PhysX
			}

			libdirs {
				CROWN_THIRD_DIR .. "luajit/src",
				"$(PHYSX_SDK_LINUX)/Lib/linux32"
			}

			postbuildcommands {
				"cp " .. CROWN_THIRD_DIR .. "luajit/src/luajit " .. CROWN_INSTALL_DIR .. "bin/linux32/",
				"cp " .. CROWN_THIRD_DIR .. "luajit/src/jit " .. CROWN_INSTALL_DIR .. "bin/linux32/" .. " -r",
			}

		configuration { "linux-*", "x64" }
			targetdir(CROWN_INSTALL_DIR .. "bin/linux64")

			libdirs {
				CROWN_THIRD_DIR .. "luajit/src",
				"$(PHYSX_SDK_LINUX)/Lib/linux64"
			}

			postbuildcommands {
				"cp " .. CROWN_THIRD_DIR .. "luajit/src/luajit " .. CROWN_INSTALL_DIR .. "bin/linux64/",
				"cp " .. CROWN_THIRD_DIR .. "luajit/src/jit " .. CROWN_INSTALL_DIR .. "bin/linux64/" .. " -r",
			}

		configuration { "android" }
			kind "ConsoleApp"
			targetprefix "lib"
			targetextension ".so"

			targetdir(CROWN_INSTALL_DIR .. "bin/android") -- must be specified by user -- tmp

			flags { "NoImportLib" }

			defines { "__STDC_FORMAT_MACROS" }

			buildoptions {
				"--sysroot=$(ANDROID_NDK_ROOT)/platforms/android-14/arch-arm",
				"-ffunction-sections",
				"-fPIC",
				"-march=armv7-a",
				"-mfloat-abi=softfp",
				"-mthumb",
				"-no-canonical-prefixes",
				"-std=c++03",
				"-Wno-psabi", -- note: the mangling of 'va_list' has changed in GCC 4.4.0
				"-no-canonical-prefixes",
				"-fstack-protector",
				"-mfpu=neon",
				"-Wa,--noexecstack",
			}
			
			linkoptions {
				"-shared",
				"-nostdlib",
				"-static-libgcc",
				"--sysroot=$(ANDROID_NDK_ROOT)/platforms/android-14/arch-arm",
				"$(ANDROID_NDK_ROOT)/platforms/android-14/arch-arm/usr/lib/crtbegin_so.o",
				"$(ANDROID_NDK_ROOT)/platforms/android-14/arch-arm/usr/lib/crtend_so.o",
				"-no-canonical-prefixes",
				"-Wl,--no-undefined",
				"-Wl,-z,noexecstack",
				"-Wl,-z,relro",
				"-Wl,-z,now",
				"-march=armv7-a",
				"-Wl,--fix-cortex-a8",
			}

			links {
				":libluajit.a",
				"android",
				"c",
				"dl",
				"EGL",
				"gcc",
				"GLESv2",
				"gnustl_static",
				"log",
				"m",
				"OpenSLES"
			}

			includedirs {
				"$(ANDROID_NDK_ROOT)/sources/cxx-stl/gnu-libstdc++/4.8/include",
				"$(ANDROID_NDK_ROOT)/sources/android/native_app_glue",
				"$(ANDROID_NDK_ROOT)/sources/cxx-stl/gnu-libstdc++/4.8/libs/armeabi-v7a/include",
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
				CROWN_SOURCE_DIR .. "engine/os/android",
				CROWN_SOURCE_DIR .. "/engine/renderers/backend/gl/egl",
				CROWN_THIRD_DIR .. "luajit/src",
				CROWN_THIRD_DIR .. "opengl",
				CROWN_THIRD_DIR .. "openal/include",
				CROWN_THIRD_DIR .. "freetype",
				CROWN_THIRD_DIR .. "stb_image",
				CROWN_THIRD_DIR .. "stb_vorbis",
			}

			libdirs {
				"$(ANDROID_NDK_ROOT)/sources/cxx-stl/gnu-libstdc++/4.8/libs/armeabi-v7a",
				"$(ANDROID_NDK_ROOT)/platforms/android-14/arch-arm/usr/lib",
				"$(PHYSX_SDK_ANDROID)/Lib/android9_neon",
				CROWN_THIRD_DIR .. "luajit/src",
			}

			excludes {
				CROWN_SOURCE_DIR .. "engine/os/linux/*",
				CROWN_SOURCE_DIR .. "engine/os/win/*",
				CROWN_SOURCE_DIR .. "engine/renderers/backend/gl/glx/*",
				CROWN_SOURCE_DIR .. "engine/renderers/backend/gl/wgl/*",
				CROWN_SOURCE_DIR .. "engine/audio/backend/al_sound_world.cpp"
			}

		configuration { "debug", "android" }
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
		configuration { "development", "android"}
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
		configuration { "release", "android"}
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
			kind "ConsoleApp"

			targetdir (CROWN_INSTALL_DIR .. "windows")

			linkoptions {
				"/ignore:4199", -- LNK4199: /DELAYLOAD:*.dll ignored; no imports found from *.dll
				"/ignore:4221", -- LNK4221: This object file does not define any previously undefined public symbols, so it will not be used by any link operation that consumes this library

			}

			defines {
				"WIN32",
				"_WIN32",
				"_HAS_EXCEPTIONS=0",
				"_HAS_ITERATOR_DEBUGGING=0",
				"_SCL_SECURE=0",
				"_SECURE_SCL=0",
				"_SCL_SECURE_NO_WARNINGS",
				"_CRT_SECURE_NO_WARNINGS",
				"_CRT_SECURE_NO_DEPRECATE"
			}

			buildoptions {
				"/Oy-", -- Suppresses creation of frame pointers on the call stack.
				"/Ob2", -- The Inline Function Expansion
			}

			links {
				"OpenGL32",
				"lua51",
				"OpenAL32"
			}

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
				CROWN_THIRD_DIR .. "luajit/src",
				CROWN_THIRD_DIR .. "opengl",
				CROWN_THIRD_DIR .. "openal/include",
				CROWN_THIRD_DIR .. "freetype",
				CROWN_THIRD_DIR .. "stb_image",
				CROWN_THIRD_DIR .. "stb_vorbis",
				CROWN_SOURCE_DIR .. "/engine/os/win",
				CROWN_SOURCE_DIR .. "/engine/renderers/backend/gl/wgl"
			}

			excludes {
				CROWN_SOURCE_DIR .. "engine/os/android/*",
				CROWN_SOURCE_DIR .. "engine/os/linux/*",
				CROWN_SOURCE_DIR .. "engine/os/posix/*",
				CROWN_SOURCE_DIR .. "engine/renderers/backend/gl/egl/*",
				CROWN_SOURCE_DIR .. "engine/renderers/backend/gl/glx/*",
				CROWN_SOURCE_DIR .. "engine/audio/backend/sles_sound_world.cpp"
			}

		configuration { "x32", "vs*" }
			libdirs {
				"$(PHYSX_SDK_WINDOWS)/Lib/win32",
				CROWN_THIRD_DIR .. "luajit/src",
				CROWN_THIRD_DIR .. "openal/lib"
			}

		configuration { "x64", "vs*" }
			defines { "_WIN64" }

			libdirs {
				"$(PHYSX_SDK_WINDOWS)/Lib/win64",
				CROWN_THIRD_DIR .. "luajit/src",
				CROWN_THIRD_DIR .. "openal/lib"
			}

		configuration { "debug", "x32", "vs*"}
			links {
				"PhysX3CharacterKinematicCHECKED_x86",
				"PhysX3CHECKED_x86",
				"PhysX3CommonCHECKED_x86",
				"PhysX3CookingCHECKED_x86",
				"PhysX3ExtensionsCHECKED"
			}

		configuration { "debug", "x64", "vs*" }
			links {
				"PhysX3CharacterKinematicCHECKED_x64",
				"PhysX3CHECKED_x64",
				"PhysX3CommonCHECKED_x64",
				"PhysX3CookingCHECKED_x64",
				"PhysX3ExtensionsCHECKED"
			}

		configuration { "development", "x32", "vs*" }
			links {
				"PhysX3CharacterKinematicPROFILE_x86",
				"PhysX3PROFILE_x86",
				"PhysX3CommonPROFILE_x86",
				"PhysX3CookingPROFILE_x86",
				"PhysX3ExtensionsPROFILE"
			}

		configuration { "development", "x64", "vs*" }
			links {
				"PhysX3CharacterKinematicPROFILE_x64",
				"PhysX3PROFILE_x64",
				"PhysX3CommonPROFILE_x64",
				"PhysX3CookingPROFILE_x64",
				"PhysX3ExtensionsPROFILE"
			}

		configuration { "release", "x32", "vs*" }
			links {
				"PhysX3CharacterKinematic_x86",
				"PhysX3_x86",
				"PhysX3Common_x86",
				"PhysX3Cooking_x86",
				"PhysX3Extensions"
			}
			
		configuration { "release", "x64", "vs*" }
			links {
				"PhysX3CharacterKinematic_x64",
				"PhysX3_x64",
				"PhysX3Common_x64",
				"PhysX3Cooking_x64",
				"PhysX3Extensions"
			}
