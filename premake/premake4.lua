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
	platforms { "native", "x32", "x64" }

	-- Avoid error invoking premake4 --help
	if _ACTION == nil then return end

	if _ACTION == "clean" then os.rmdir(CROWN_BUILD_DIR) end

	if _ACTION == "gmake" then

		if _OPTIONS["compiler"] == "linux-gcc" then

			if not os.is("linux") then print("Action not valid in current OS.") end
			location(CROWN_BUILD_DIR .. "linux")

		elseif _OPTIONS["compiler"] == "android" then

			if not os.getenv("ANDROID_NDK_ROOT") then print("Set ANDROID_NDK_ROOT environment variable.") end
			if not os.getenv("ANDROID_NDK_ARM") then print("Set ANDROID_NDK_ARM environment variables.") end

			premake.gcc.cc = "$(ANDROID_NDK_ARM)/bin/arm-linux-androideabi-gcc"
			premake.gcc.cxx = "$(ANDROID_NDK_ARM)/bin/arm-linux-androideabi-g++"
			premake.gcc.ar = "$(ANDROID_NDK_ARM)/bin/arm-linux-androideabi-ar"

			location(CROWN_BUILD_DIR .. "android")

		end
	end

	if _ACTION == "vs2010" or _ACTION == "vs2008" then

		if not os.is("windows") then print("Action not valid in current OS.") end
		if not os.getenv("DXSDK_DIR") then print("Environment variable DXSDK_DIR must be set.") end
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
				"-Wextra",
				-- "-Werror",
				"-pedantic",
				"-Wno-unknown-pragmas",
				"-Wno-unused-local-typedefs"
			}
			
			linkoptions {
				"-Wl,-rpath=\\$$ORIGIN"
			}

			links {
				"Xrandr",
				"pthread",
				"dl",
				"GL",
				"X11",
				"openal",
				"luajit-5.1"
			}

			includedirs {
				CROWN_SOURCE_DIR .. "/engine/os/linux",
				CROWN_SOURCE_DIR .. "/engine/renderers/backend/gl/glx",
			}

			excludes {
				CROWN_SOURCE_DIR .. "engine/os/android/*",
				CROWN_SOURCE_DIR .. "engine/os/win/*",
				CROWN_SOURCE_DIR .. "engine/renderers/backend/gl/egl/*",
				CROWN_SOURCE_DIR .. "engine/renderers/backend/gl/wgl/*",
				CROWN_SOURCE_DIR .. "engine/audio/backend/SLESSoundWorld.cpp",
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

			libdirs {
				CROWN_THIRD_DIR .. "luajit/x86/lib",
				CROWN_THIRD_DIR .. "physx/x86/lib"
			}

			postbuildcommands {
				"cp " .. CROWN_THIRD_DIR .. "luajit/x86/bin/luajit " .. CROWN_INSTALL_DIR .. "bin/linux32/",
				"cp " .. CROWN_THIRD_DIR .. "luajit/x86/lib/libluajit-5.1.so.2 " .. CROWN_INSTALL_DIR .. "bin/linux32/",
				"cp -r " .. CROWN_THIRD_DIR .. "/luajit/x86/share/luajit-2.0.3/jit " .. CROWN_INSTALL_DIR .. "bin/linux32/"
			}

		configuration { "linux-*", "x64" }
			targetdir(CROWN_INSTALL_DIR .. "bin/linux64")

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
				CROWN_THIRD_DIR .. "stb_vorbis",
			}

			libdirs {
				CROWN_THIRD_DIR .. "luajit/x86_64/lib",
				CROWN_THIRD_DIR .. "physx/x86_64/lib",
			}

			postbuildcommands {
				"cp " .. CROWN_THIRD_DIR .. "luajit/x86_64/bin/luajit " .. CROWN_INSTALL_DIR .. "bin/linux64/",
				"cp " .. CROWN_THIRD_DIR .. "luajit/x86_64/lib/libluajit-5.1.so.2 " .. CROWN_INSTALL_DIR .. "bin/linux64/",
				"cp -r " .. CROWN_THIRD_DIR .. "/luajit/x86_64/share/luajit-2.0.3/jit " .. CROWN_INSTALL_DIR .. "bin/linux64/"
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
				":libluajit-5.1.a",
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
				CROWN_SOURCE_DIR .. "engine/os/android",
				CROWN_SOURCE_DIR .. "/engine/renderers/backend/gl/egl",
				CROWN_THIRD_DIR .. "luajit/android/include/luajit-2.0",
				CROWN_THIRD_DIR .. "physx/android/include",
				CROWN_THIRD_DIR .. "physx/android/include/common",
				CROWN_THIRD_DIR .. "physx/android/include/characterkinematic",
				CROWN_THIRD_DIR .. "physx/android/include/cloth",
				CROWN_THIRD_DIR .. "physx/android/include/common",
				CROWN_THIRD_DIR .. "physx/android/include/cooking",
				CROWN_THIRD_DIR .. "physx/android/include/extensions",
				CROWN_THIRD_DIR .. "physx/android/include/foundation",
				CROWN_THIRD_DIR .. "physx/android/include/geometry",
				CROWN_THIRD_DIR .. "physx/android/include/particles",
				CROWN_THIRD_DIR .. "physx/android/include/physxprofilesdk",
				CROWN_THIRD_DIR .. "physx/android/include/physxvisualdebuggersdk",
				CROWN_THIRD_DIR .. "physx/android/include/pvd",
				CROWN_THIRD_DIR .. "physx/android/include/pxtask",
				CROWN_THIRD_DIR .. "physx/android/include/RepX",
				CROWN_THIRD_DIR .. "physx/android/include/RepXUpgrader",
				CROWN_THIRD_DIR .. "physx/android/include/vehicle",
				CROWN_THIRD_DIR .. "opengl",
				CROWN_THIRD_DIR .. "openal/include",
				CROWN_THIRD_DIR .. "freetype",
				CROWN_THIRD_DIR .. "stb_image",
				CROWN_THIRD_DIR .. "stb_vorbis",
				"$(ANDROID_NDK_ROOT)/sources/cxx-stl/gnu-libstdc++/4.8/include",
				"$(ANDROID_NDK_ROOT)/sources/android/native_app_glue",
				"$(ANDROID_NDK_ROOT)/sources/cxx-stl/gnu-libstdc++/4.8/libs/armeabi-v7a/include"
			}

			libdirs {
				CROWN_THIRD_DIR .. "luajit/android/lib",
				CROWN_THIRD_DIR .. "physx/android/lib",
				"$(ANDROID_NDK_ROOT)/sources/cxx-stl/gnu-libstdc++/4.8/libs/armeabi-v7a",
				"$(ANDROID_NDK_ROOT)/platforms/android-14/arch-arm/usr/lib"
			}

			excludes {
				CROWN_SOURCE_DIR .. "engine/os/linux/*",
				CROWN_SOURCE_DIR .. "engine/os/win/*",
				CROWN_SOURCE_DIR .. "engine/renderers/backend/gl/glx/*",
				CROWN_SOURCE_DIR .. "engine/renderers/backend/gl/wgl/*",
				CROWN_SOURCE_DIR .. "engine/audio/backend/ALSoundWorld.cpp"
			}

		configuration { "debug", "android" }
			-- linkoptions
			-- {
			-- 	"-Wl,--start-group $(addprefix -l," ..
			-- 	"	LowLevelClothCHECKED" ..
			-- 	"	PhysX3CHECKED " ..
			-- 	"	PhysX3CommonCHECKED" ..
			-- 	"	PxTaskCHECKED" ..
			-- 	"	LowLevelCHECKED" ..
			-- 	"	PhysX3CharacterKinematicCHECKED" ..
			-- 	"	PhysX3CookingCHECKED" ..
			-- 	"	PhysX3ExtensionsCHECKED" ..
			-- 	"	PhysX3VehicleCHECKED" ..
			-- 	"	PhysXProfileSDKCHECKED" ..
			-- 	"	PhysXVisualDebuggerSDKCHECKED" ..
			-- 	"	PvdRuntimeCHECKED" ..
			-- 	"	SceneQueryCHECKED" ..
			-- 	"	SimulationControllerCHECKED" ..
			-- 	") -Wl,--end-group"
			-- }
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
			-- linkoptions
			-- { 
			-- 	"-Wl,--start-group $(addprefix -l," ..
			-- 	"	LowLevelClothPROFILE" ..
			-- 	"	PhysX3PROFILE " ..
			-- 	"	PhysX3CommonPROFILE" ..
			-- 	"	PxTaskPROFILE" ..
			-- 	"	LowLevelPROFILE" ..
			-- 	"	PhysX3CharacterKinematicPROFILE" ..
			-- 	"	PhysX3CookingPROFILE" ..
			-- 	"	PhysX3ExtensionsPROFILE" ..
			-- 	"	PhysX3VehiclePROFILE" ..
			-- 	"	PhysXProfileSDKPROFILE" ..
			-- 	"	PhysXVisualDebuggerSDKPROFILE" ..
			-- 	"	PvdRuntimePROFILE" ..
			-- 	"	SceneQueryPROFILE" ..
			-- 	"	SimulationControllerPROFILE" ..
			-- 	") -Wl,--end-group"
			-- }
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

		-- it's necessary to define DXSDK_DIR env variable to DirectX sdk directory
		configuration { "vs*" }
			kind "ConsoleApp"

			targetdir (CROWN_INSTALL_DIR .. "windows")

			linkoptions {
				"/ignore:4199", -- LNK4199: /DELAYLOAD:*.dll ignored; no imports found from *.dll
				"/ignore:4221", -- LNK4221: This object file does not define any previously undefined public symbols, so it will not be used by any link operation that consumes this library

			}
			links { -- this is needed only for testing with GLES2/3 on Windows with VS2008
				"DelayImp",
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
				CROWN_SOURCE_DIR .. "/engine/os/win",
				CROWN_SOURCE_DIR .. "/engine/renderers/backend/gl/wgl"
			}

			libdirs {
				CROWN_THIRD_DIR .. "openal/lib"
			}

			excludes {
				CROWN_SOURCE_DIR .. "engine/os/android/*",
				CROWN_SOURCE_DIR .. "engine/os/linux/*",
				CROWN_SOURCE_DIR .. "engine/os/posix/*",
				CROWN_SOURCE_DIR .. "engine/renderers/backend/gl/egl/*",
				CROWN_SOURCE_DIR .. "engine/renderers/backend/gl/glx/*",
				CROWN_SOURCE_DIR .. "engine/audio/backend/SLESSoundWorld.cpp"
			}

		configuration { "vs2010" }
			linkoptions { 
				"/DELAYLOAD:\"libEGL.dll\"", -- this is needed only for testing with GLES2/3 on Windows with VS201x
				"/DELAYLOAD:\"libGLESv2.dll\""
			}

		configuration { "vs*", "debug" }
			links {
				"PhysX3ExtensionsCHECKED",
				"PhysXProfileSDKCHECKED",
				"PhysXVisualDebuggerSDKCHECKED",
				"PxTaskCHECKED"
			}

		configuration { "vs*", "development" }
			links {
				"PhysX3ExtensionsPROFILE",
				"PhysXProfileSDKPROFILE",
				"PhysXVisualDebuggerSDKPROFILE",
				"PxTaskPROFILE"
			}

		configuration { "vs*", "release" }
			links {
				"PhysX3Extensions",
				"PhysXProfileSDK",
				"PhysXVisualDebuggerSDK",
				"PxTask",
			}

		configuration { "x32", "vs*" }
			includedirs {
				CROWN_THIRD_DIR .. "luajit/win32/include/luajit-2.0",
				CROWN_THIRD_DIR .. "physx/win32/include",
				CROWN_THIRD_DIR .. "physx/win32/include/common",
				CROWN_THIRD_DIR .. "physx/win32/include/characterkinematic",
				CROWN_THIRD_DIR .. "physx/win32/include/cloth",
				CROWN_THIRD_DIR .. "physx/win32/include/common",
				CROWN_THIRD_DIR .. "physx/win32/include/cooking",
				CROWN_THIRD_DIR .. "physx/win32/include/extensions",
				CROWN_THIRD_DIR .. "physx/win32/include/foundation",
				CROWN_THIRD_DIR .. "physx/win32/include/geometry",
				CROWN_THIRD_DIR .. "physx/win32/include/particles",
				CROWN_THIRD_DIR .. "physx/win32/include/physxprofilesdk",
				CROWN_THIRD_DIR .. "physx/win32/include/physxvisualdebuggersdk",
				CROWN_THIRD_DIR .. "physx/win32/include/pvd",
				CROWN_THIRD_DIR .. "physx/win32/include/pxtask",
				CROWN_THIRD_DIR .. "physx/win32/include/RepX",
				CROWN_THIRD_DIR .. "physx/win32/include/RepXUpgrader",
				CROWN_THIRD_DIR .. "physx/win32/include/vehicle",
				CROWN_THIRD_DIR .. "opengl",
				CROWN_THIRD_DIR .. "openal/include",
				CROWN_THIRD_DIR .. "freetype",
				CROWN_THIRD_DIR .. "stb_image",
				CROWN_THIRD_DIR .. "stb_vorbis"
			}

			libdirs {
				CROWN_THIRD_DIR .. "luajit/win32/lib",
				CROWN_THIRD_DIR .. "physx/win32/lib"
			}

		configuration { "x64", "vs*" }
			defines { "_WIN64" }

			includedirs {
				CROWN_THIRD_DIR .. "luajit/win64/include/luajit-2.0",
				CROWN_THIRD_DIR .. "physx/win64/include",
				CROWN_THIRD_DIR .. "physx/win64/include/common",
				CROWN_THIRD_DIR .. "physx/win64/include/characterkinematic",
				CROWN_THIRD_DIR .. "physx/win64/include/cloth",
				CROWN_THIRD_DIR .. "physx/win64/include/common",
				CROWN_THIRD_DIR .. "physx/win64/include/cooking",
				CROWN_THIRD_DIR .. "physx/win64/include/extensions",
				CROWN_THIRD_DIR .. "physx/win64/include/foundation",
				CROWN_THIRD_DIR .. "physx/win64/include/geometry",
				CROWN_THIRD_DIR .. "physx/win64/include/particles",
				CROWN_THIRD_DIR .. "physx/win64/include/physxprofilesdk",
				CROWN_THIRD_DIR .. "physx/win64/include/physxvisualdebuggersdk",
				CROWN_THIRD_DIR .. "physx/win64/include/pvd",
				CROWN_THIRD_DIR .. "physx/win64/include/pxtask",
				CROWN_THIRD_DIR .. "physx/win64/include/RepX",
				CROWN_THIRD_DIR .. "physx/win64/include/RepXUpgrader",
				CROWN_THIRD_DIR .. "physx/win64/include/vehicle",
				CROWN_THIRD_DIR .. "opengl",
				CROWN_THIRD_DIR .. "openal/include",
				CROWN_THIRD_DIR .. "freetype",
				CROWN_THIRD_DIR .. "stb_image",
				CROWN_THIRD_DIR .. "stb_vorbis"
			}

			libdirs {
				CROWN_THIRD_DIR .. "luajit/win64/lib",
				CROWN_THIRD_DIR .. "physx/win64/lib"
			}

		configuration { "debug", "x32", "vs*"}
			links {
				"PhysX3CharacterKinematicCHECKED_x86",
				"PhysX3CHECKED_x86",
				"PhysX3CommonCHECKED_x86",
				"PhysX3CookingCHECKED_x86"
			}

		configuration { "debug", "x64", "vs*" }
			links {
				"PhysX3CharacterKinematicCHECKED_x64",
				"PhysX3CHECKED_x64",
				"PhysX3CommonCHECKED_x64",
				"PhysX3CookingCHECKED_x64"
			}

		configuration { "development", "x32", "vs*" }
			links {
				"PhysX3CharacterKinematicPROFILE_x86",
				"PhysX3PROFILE_x86",
				"PhysX3CommonPROFILE_x86",
				"PhysX3CookingPROFILE_x86"
			}

		configuration { "development", "x64", "vs*" }
			links {
				"PhysX3CharacterKinematicPROFILE_x64",
				"PhysX3PROFILE_x64",
				"PhysX3CommonPROFILE_x64",
				"PhysX3CookingPROFILE_x64"
			}

		configuration { "debug", "x32", "vs*" }
			links {
				"PhysX3CharacterKinematic_x86",
				"PhysX3_x86",
				"PhysX3Common_x86",
				"PhysX3Cooking_x86"
			}
			
		configuration { "debug", "x64", "vs*" }
			links {
				"PhysX3CharacterKinematic_x64",
				"PhysX3_x64",
				"PhysX3Common_x64",
				"PhysX3Cooking_x64"
			}
