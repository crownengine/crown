--
-- Copyright (c) 2012-2016 Daniele Bartolini and individual contributors.
-- License: https://github.com/taylor001/crown/blob/master/LICENSE
--

function toolchain(build_dir, lib_dir)

	newoption
	{
		trigger = "compiler",
		value = "COMPILER",
		description = "Choose compiler",
		allowed =
		{
			{ "android-arm", "Android - ARM"        },
			{ "linux-gcc",   "Linux (GCC compiler)" }
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

		if "linux-gcc" == _OPTIONS["compiler"] then

			if not os.is("linux") then print("Action not valid in current OS.") end

			location(build_dir .. "projects/" .. "linux")
		end

		if "android-arm" == _OPTIONS["compiler"] then

			if not os.getenv("ANDROID_NDK_ROOT") then
				print("Set ANDROID_NDK_ROOT environment variable.")
			end

			if not os.getenv("ANDROID_NDK_ARM") then
				print("Set ANDROID_NDK_ARM environment variables.")
			end

			premake.gcc.cc = "$(ANDROID_NDK_ARM)/bin/arm-linux-androideabi-gcc"
			premake.gcc.cxx = "$(ANDROID_NDK_ARM)/bin/arm-linux-androideabi-g++"
			premake.gcc.ar = "$(ANDROID_NDK_ARM)/bin/arm-linux-androideabi-ar"
			location(build_dir .. "projects/" .. "android")
		end
	end

	if _ACTION == "vs2013" then

		if not os.is("windows") then print("Action not valid in current OS.") end

		if not os.getenv("DXSDK_DIR") then
			print("Set DXSDK_DIR environment variable.")
		end

		location(build_dir .. "projects/" .. _ACTION)
	end

	flags {
		"StaticRuntime",
		"NoPCH",
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

	configuration { "development or release" }
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

	configuration { "debug or development", "linux-*" }
		linkoptions {
			"-rdynamic"
		}

	configuration { "x32", "linux-*" }
		targetdir (build_dir .. "linux32" .. "/bin")
		objdir (build_dir .. "linux32" .. "/obj")
		libdirs {
			lib_dir .. "../.build/linux32/bin",
		}
		buildoptions {
			"-m32",
			"-malign-double", -- Required by PhysX
		}

	configuration { "x64", "linux-*" }
		targetdir (build_dir .. "linux64" .. "/bin")
		objdir (build_dir .. "linux64" .. "/obj")
		libdirs {
			lib_dir .. "../.build/linux64/bin",
		}
		buildoptions {
			"-m64",
		}

	configuration { "linux-*" }
		buildoptions {
			"-Wall",
			"-Wextra",
			"-msse2",
			-- "-Werror",
			-- "-pedantic",
			"-Wno-unknown-pragmas",
			"-Wno-unused-local-typedefs",
		}
		buildoptions_cpp {
			"-std=c++11",
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
			"-no-canonical-prefixes",
			"-Wa,--noexecstack",
			"-fstack-protector",
			"-ffunction-sections",
			"-Wno-psabi", -- note: the mangling of 'va_list' has changed in GCC 4.4.0
			"-Wunused-value",
		}
		buildoptions_cpp {
			"-std=c++11",
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
			lib_dir .. "../.build/android-arm/bin",
			"$(ANDROID_NDK_ROOT)/sources/cxx-stl/gnu-libstdc++/4.8/libs/armeabi-v7a",
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
		}
		linkoptions {
			"--sysroot=$(ANDROID_NDK_ROOT)/platforms/android-14/arch-arm",
			"$(ANDROID_NDK_ROOT)/platforms/android-14/arch-arm/usr/lib/crtbegin_so.o",
			"$(ANDROID_NDK_ROOT)/platforms/android-14/arch-arm/usr/lib/crtend_so.o",
			"-march=armv7-a",
			"-Wl,--fix-cortex-a8",
		}

	configuration { "vs*" }
		includedirs { CROWN_DIR .. "src/core/compat/msvc" }
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
			"NOMINMAX",
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
			"$(DXSDK_DIR)/Lib/x86",
		}

	configuration { "x64", "vs*" }
		targetdir (build_dir .. "win64" .. "/bin")
		objdir (build_dir .. "win64" .. "/obj")
		libdirs {
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
