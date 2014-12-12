--
-- Copyright (c) 2012-2014 Daniele Bartolini and individual contributors.
-- License: https://github.com/taylor001/crown/blob/master/LICENSE
--

function toolchain(build_dir, lib_dir)

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
			{ "android-arm", "Android - ARM"          },
			{ "linux-gcc",   "Linux (GCC compiler)"   },
			{ "osx-clang", "OSX (clang compiler)"	  }
		}
	}

	if (_ACTION == nil) then return end

	location (build_dir .. "projects/" .. _ACTION)

	if _ACTION == "clean" then
		os.rmdir(BUILD_DIR)
	end

	if _ACTION == "gmake" then

		if nil == _OPTIONS["compiler"] then
			print("Choose a compiler!")
			os.exit(1)
		end

		if "osx-clang" == _OPTIONS["compiler"] then

			if not os.getenv("PHYSX_SDK_OSX") then
				print("Set PHYSX_SDK_OSX environment variable.")
			end

			location(build_dir .. "projects/" .. "osx")
		end

		if "linux-gcc" == _OPTIONS["compiler"] then

			if not os.is("linux") then print("Action not valid in current OS.") end

			if not os.getenv("PHYSX_SDK_LINUX") then
				print("Set PHYSX_SDK_LINUX environment variable.")
			end

			location(build_dir .. "projects/" .. "linux")
		end

		if "android-arm" == _OPTIONS["compiler"] then

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
			location(build_dir .. "projects/" .. "android")
		end
	end

	if _ACTION == "vs2012" then

		if not os.is("windows") then print("Action not valid in current OS.") end

		if not os.getenv("PHYSX_SDK_WINDOWS") then
			print("Set PHYSX_SDK_WINDOWS environment variable.")
		end
		if not os.getenv("DXSDK_DIR") then
			print("Set DXSDK_DIR environment variable.")
		end

		location(build_dir .. "projects/" .. "windows")
	end

	flags {
		"StaticRuntime",
		"NoPCH",
		"NativeWChar",
		"NoRTTI",
		"NoExceptions",
		"NoEditAndContinue",
		"Symbols",
	}

	defines {
		"__STDC_FORMAT_MACROS",
		"__STDC_CONSTANT_MACROS",
		"__STDC_LIMIT_MACROS",
	}

	configuration { "development" }
		flags {
			"OptimizeSpeed"
		}

	configuration { "release" }
		flags {
			"OptimizeSpeed"
		}

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

	configuration { "x32", "osx-*" }
		targetdir (build_dir .. "osx32" .. "/bin")
		objdir (build_dir .. "osx32" .. "/obj")
		libdirs {
			"/usr/local/lib",
			lib_dir .. "../.build/osx32/bin",
			lib_dir .. "bgfx/.build/osx32_clang/bin",
			"$(PHYSX_SDK_OSX)/Lib/osx32",
		}
		buildoptions {
			"-m32",
		}

	configuration { "x64", "osx-*" }
		targetdir (build_dir .. "osx64" .. "/bin")
		objdir (build_dir .. "osx64" .. "/obj")
		libdirs {
			lib_dir .. "luajit/src",
			lib_dir .. "../.build/osx64/bin",
			lib_dir .. "bgfx/.build/osx64_clang/bin",
			"$(PHYSX_SDK_LINUX)/Lib/linux64",
		}
		buildoptions {
			"-m64",
		}

	configuration { "x32", "linux-*" }
		targetdir (build_dir .. "linux32" .. "/bin")
		objdir (build_dir .. "linux32" .. "/obj")
		libdirs {
			lib_dir .. "luajit/src",
			lib_dir .. "../.build/linux32/bin",
			lib_dir .. "bgfx/.build/linux32_gcc/bin",
			"$(PHYSX_SDK_LINUX)/Lib/linux32",
		}
		buildoptions {
			"-m32",
			"-malign-double", -- Required by PhysX
		}

	configuration { "x64", "linux-*" }
		targetdir (build_dir .. "linux64" .. "/bin")
		objdir (build_dir .. "linux64" .. "/obj")
		libdirs {
			lib_dir .. "luajit/src",
			lib_dir .. "../.build/linux64/bin",
			lib_dir .. "bgfx/.build/linux64_gcc/bin",
			"$(PHYSX_SDK_LINUX)/Lib/linux64"
		}
		buildoptions {
			"-m64",
		}

	configuration { "i386" }

	configuration { "osx-*" }
		buildoptions {
			"-g",
			"-Wfatal-errors",
			"-msse2",
			"-Wunused-value",
			"-Wundef",
		}
		includedirs { lib_dir .. "bx/include/compat/osx" }

	configuration { "linux-*" }
		buildoptions {
			"-std=c++03",
			"-Wall",
			"-Wextra",
			-- "-Werror",
			-- "-pedantic",
			"-Wno-unknown-pragmas",
			"-Wno-unused-local-typedefs",
		}
		linkoptions {
			"-Wl,-rpath=\\$$ORIGIN",
			"-Wl,--no-as-needed",
		}

	configuration { "android-*" }
		flags {
			"NoImportLib"
		}
		includedirs {
			"$(ANDROID_NDK_ROOT)/sources/cxx-stl/gnu-libstdc++/4.8/include",
			"$(ANDROID_NDK_ROOT)/sources/android/native_app_glue",
		}
		linkoptions {
			"-nostdlib",
			"-static-libgcc",
		}
		links {
			":libluajit.a",
			"bgfxDebug",
			"c",
			"dl",
			"m",
			"android",
			"log",
			"gnustl_static",
			"gcc",
		}
		buildoptions {
			"-fPIC",
			"-std=c++03",
			"-no-canonical-prefixes",
			"-Wa,--noexecstack",
			"-fstack-protector",
			"-ffunction-sections",
			"-Wno-psabi", -- note: the mangling of 'va_list' has changed in GCC 4.4.0
			"-Wunused-value",
			-- "-Wundef", -- note: avoids PhysX warnings
		}
		linkoptions {
			"-no-canonical-prefixes",
			"-Wl,--no-undefined",
			"-Wl,-z,noexecstack",
			"-Wl,-z,relro",
			"-Wl,-z,now",
		}

	configuration { "android-arm" }
		targetdir (build_dir .. "android-arm" .. "/bin")
		objdir (build_dir .. "android-arm" .. "/obj")
		libdirs {
			lib_dir .. "luajit/src",
			lib_dir .. "../.build/android-arm/bin",
			lib_dir .. "bgfx/.build/android-arm/bin",
			"$(ANDROID_NDK_ROOT)/sources/cxx-stl/gnu-libstdc++/4.8/libs/armeabi-v7a",
			"$(PHYSX_SDK_ANDROID)/Lib/android9_neon",
		}
		includedirs {
			"$(ANDROID_NDK_ROOT)/sources/cxx-stl/gnu-libstdc++/4.8/libs/armeabi-v7a/include",
		}
		buildoptions {
			"--sysroot=$(ANDROID_NDK_ROOT)/platforms/android-14/arch-arm",
			"-mthumb",
			"-march=armv7-a",
			"-mfloat-abi=softfp",
			"-mfpu=neon",
			"-Wunused-value",
			-- "-Wundef", -- note: avoids PhysX warnings
		}
		linkoptions {
			"--sysroot=$(ANDROID_NDK_ROOT)/platforms/android-14/arch-arm",
			"$(ANDROID_NDK_ROOT)/platforms/android-14/arch-arm/usr/lib/crtbegin_so.o",
			"$(ANDROID_NDK_ROOT)/platforms/android-14/arch-arm/usr/lib/crtend_so.o",
			"-march=armv7-a",
			"-Wl,--fix-cortex-a8",
		}

	configuration { "vs*" }
		includedirs { CROWN_DIR .. "engine/core/compat/msvc" }
		defines {
			"WIN32",
			"_WIN32",
			"_HAS_EXCEPTIONS=0",
			"_HAS_ITERATOR_DEBUGGING=0",
			"_SCL_SECURE=0",
			"_SECURE_SCL=0",
			"_SCL_SECURE_NO_WARNINGS",
			"_CRT_SECURE_NO_WARNINGS",
			"_CRT_SECURE_NO_DEPRECATE",
		}
		buildoptions {
			"/Oy-", -- Suppresses creation of frame pointers on the call stack.
			"/Ob2", -- The Inline Function Expansion
		}
		linkoptions {
			"/ignore:4199", -- LNK4199: /DELAYLOAD:*.dll ignored; no imports found from *.dll
			"/ignore:4221", -- LNK4221: This object file does not define any previously undefined public symbols, so it will not be used by any link operation that consumes this library
			"/ignore:4099", -- LNK4099: The linker was unable to find your .pdb file.
		}

	configuration { "x32", "vs*" }
		targetdir (build_dir .. "win32" .. "/bin")
		objdir (build_dir .. "win32" .. "/obj")
		libdirs {
			lib_dir .. "luajit/src",
			lib_dir .. "openal/lib",
			lib_dir .. "bgfx/.build/win32_vs2013/bin",
			"$(PHYSX_SDK_WINDOWS)/Lib/win32",
			"$(DXSDK_DIR)/Lib/x86",
		}

	configuration { "x64", "vs*" }
		targetdir (build_dir .. "win64" .. "/bin")
		objdir (build_dir .. "win64" .. "/obj")
		libdirs {
			lib_dir .. "luajit/src",
			lib_dir .. "openal/lib",
			lib_dir .. "bgfx/.build/win64_vs2013/bin",
			"$(PHYSX_SDK_WINDOWS)/Lib/win64",
			"$(DXSDK_DIR)/Lib/x64",
		}

	configuration {} -- reset configuration
end

function strip()

	configuration { "android-arm", "release"}
		postbuildcommands {
			"$(SILENT) echo Stripping symbols",
			"$(SILENT) $(ANDROID_NDK_ARM)/bin/arm-linux-androideabi-strip -s \"$(TARGET)\""
		}

	configuration { "linux-*", "release" }
		postbuildcommands {
			"$(SILENT) echo Stripping symbols",
			"$(SILENT) strip -s \"$(TARGET)\""
		}

	configuration {} -- reset configuration
end
