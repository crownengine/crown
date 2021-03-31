--
-- Copyright (c) 2012-2021 Daniele Bartolini et al.
-- License: https://github.com/dbartolini/crown/blob/master/LICENSE
--

function toolchain(build_dir, lib_dir)

	newoption
	{
		trigger = "compiler",
		value = "COMPILER",
		description = "Choose compiler",
		allowed =
		{
			{ "android-arm",   "Android - ARM"          },
			{ "android-arm64", "Android - ARM64"        },
			{ "linux-gcc",     "Linux (GCC compiler)"   },
			{ "linux-clang",   "Linux (Clang compiler)" },
			{ "mingw-gcc",     "MinGW (GCC compiler)"   },
		}
	}

	if (_ACTION == nil) then return end

	if _ACTION == "clean" then
		os.rmdir(BUILD_DIR)
	end

	if _ACTION == "gmake" then

		if nil == _OPTIONS["compiler"] then
			print("Choose a compiler!")
			os.exit(1)
		end

		if "android-arm" == _OPTIONS["compiler"] then

			if not os.getenv("ANDROID_NDK_ABI") then
				print("Set ANDROID_NDK_ABI environment variable.")
			end

			if not os.getenv("ANDROID_NDK_ROOT") then
				print("Set ANDROID_NDK_ROOT environment variable.")
			end

			premake.gcc.cc  = "$(ANDROID_NDK_ROOT)/toolchains/llvm/prebuilt/linux-x86_64/bin/armv7a-linux-androideabi$(ANDROID_NDK_ABI)-clang"
			premake.gcc.cxx = "$(ANDROID_NDK_ROOT)/toolchains/llvm/prebuilt/linux-x86_64/bin/armv7a-linux-androideabi$(ANDROID_NDK_ABI)-clang++"
			premake.gcc.ar  = "$(ANDROID_NDK_ROOT)/toolchains/llvm/prebuilt/linux-x86_64/bin/arm-linux-androideabi-ar"

			premake.gcc.llvm = true
			location(build_dir .. "projects/android-arm")

		elseif "android-arm64" == _OPTIONS["compiler"] then

			if not os.getenv("ANDROID_NDK_ABI") then
				print("Set ANDROID_NDK_ABI environment variable.")
			end

			if not os.getenv("ANDROID_NDK_ROOT") then
				print("Set ANDROID_NDK_ROOT environment variable.")
			end

			premake.gcc.cc  = "$(ANDROID_NDK_ROOT)/toolchains/llvm/prebuilt/linux-x86_64/bin/aarch64-linux-android$(ANDROID_NDK_ABI)-clang"
			premake.gcc.cxx = "$(ANDROID_NDK_ROOT)/toolchains/llvm/prebuilt/linux-x86_64/bin/aarch64-linux-android$(ANDROID_NDK_ABI)-clang++"
			premake.gcc.ar  = "$(ANDROID_NDK_ROOT)/toolchains/llvm/prebuilt/linux-x86_64/bin/aarch64-linux-android-ar"

			premake.gcc.llvm = true
			location(build_dir .. "projects/android-arm64")

		elseif "linux-gcc" == _OPTIONS["compiler"] then

			if not os.is("linux") then
				print("Action not valid in current OS.")
			end

			location(build_dir .. "projects/linux")

		elseif "linux-clang" == _OPTIONS["compiler"] then

			if not os.is("linux") then
				print("Action not valid in current OS.")
			end

			premake.gcc.cc  = "clang"
			premake.gcc.cxx = "clang++"
			premake.gcc.ar  = "ar"
			location(build_dir .. "projects/linux-clang")

		elseif "mingw-gcc" == _OPTIONS["compiler"] then

			if not os.getenv("MINGW") then
				print("Set MINGW environment variable.")
				os.exit(1)
			end

			premake.gcc.cc  = "$(MINGW)/bin/x86_64-w64-mingw32-gcc"
			premake.gcc.cxx = "$(MINGW)/bin/x86_64-w64-mingw32-g++"
			premake.gcc.ar  = "$(MINGW)/bin/ar"
			premake.valac.cc  = premake.gcc.cc
			location(build_dir .. "projects/mingw")

		end
	elseif _ACTION == "vs2017"
		or _ACTION == "vs2019"
		then

		if not os.is("windows") then
			print("Action not valid in current OS.")
		end

		local windowsPlatform = string.gsub(os.getenv("WindowsSDKVersion") or "8.1", "\\", "")
		local action = premake.action.current()
		action.vstudio.windowsTargetPlatformVersion    = windowsPlatform
		action.vstudio.windowsTargetPlatformMinVersion = windowsPlatform

		location(build_dir .. "projects/" .. _ACTION)
	end

	flags {
		"StaticRuntime",
		"NoPCH",
		"NoRTTI",
		"NoExceptions",
		"NoEditAndContinue",
		"NoFramePointer",
		"Symbols",
	}

	defines {
		"__STDC_CONSTANT_MACROS",
		"__STDC_FORMAT_MACROS",
		"__STDC_LIMIT_MACROS",
	}

	configuration { "debug" }
		targetsuffix "-debug"

	configuration { "development" }
		targetsuffix "-development"

	configuration { "release" }
		targetsuffix "-release"

	configuration { "development or release" }
		flags {
			"NoBufferSecurityCheck",
			"OptimizeSpeed",
		}

	configuration { "debug or development", "linux-*" }
		linkoptions {
			"-rdynamic"
		}

	configuration { "linux-gcc or android-arm" }
		buildoptions {
			"-Werror=return-type",
		}

	configuration { "x32", "linux-gcc" }
		targetdir (build_dir .. "linux32" .. "/bin")
		objdir (build_dir .. "linux32" .. "/obj")
		libdirs {
			lib_dir .. "../build/linux32/bin",
		}

	configuration { "x64", "linux-gcc" }
		targetdir (build_dir .. "linux64" .. "/bin")
		objdir (build_dir .. "linux64" .. "/obj")
		libdirs {
			lib_dir .. "../build/linux64/bin",
		}

	configuration { "x64", "linux-clang" }
		targetdir (build_dir .. "linux64_clang" .. "/bin")
		objdir (build_dir .. "linux64_clang" .. "/obj")
		libdirs {
			lib_dir .. "../build/linux64_clang/bin",
		}

	configuration { "x32", "vs*" }
		targetdir (build_dir .. "windows32" .. "/bin")
		objdir (build_dir .. "windows32" .. "/obj")
		libdirs {
			lib_dir .. "../build/windows32/bin",
		}

	configuration { "x64", "vs*" }
		targetdir (build_dir .. "windows64" .. "/bin")
		objdir (build_dir .. "windows64" .. "/obj")
		libdirs {
			lib_dir .. "../build/windows64/bin",
		}

	configuration { "linux-gcc* or linux-clang" }
		buildoptions {
			"-Wall",
			"-Wextra",
			"-Wundef",
			"-msse2",
		}
		buildoptions_cpp {
			"-std=c++14",
		}
		links {
			"dl",
		}
		linkoptions {
			"-Wl,-rpath=\\$$ORIGIN",
			"-Wl,--no-as-needed",
			"-no-pie",
		}

	configuration { "android-*" }
		targetprefix("lib")
		flags {
			"NoImportLib"
		}
		includedirs {
			"$(ANDROID_NDK_ROOT)/sources/android/native_app_glue",
		}
		linkoptions {
			"-nostdlib",
		}
		links {
			"c",
			"dl",
			"m",
			"android",
			"log",
			"c++_shared",
			"gcc",
		}
		buildoptions {
			"-fPIC",
			"-no-canonical-prefixes",
			"-Wa,--noexecstack",
			"-fstack-protector-strong",
			"-ffunction-sections",
			"-Wunused-value",
			"-Wundef",
		}
		linkoptions {
			"-no-canonical-prefixes",
			"-Wl,--no-undefined",
			"-Wl,-z,noexecstack",
			"-Wl,-z,relro",
			"-Wl,-z,now",
		}

	configuration { "android-arm" }
		targetdir (build_dir .. "android-arm/bin")
		objdir (build_dir .. "android-arm/obj")
		libdirs {
			lib_dir .. "../build/android-arm/bin",
			"$(ANDROID_NDK_ROOT)/sources/cxx-stl/llvm-libc++/libs/armeabi-v7a",
		}
		includedirs {
			"$(ANDROID_NDK_ROOT)/sysroot/usr/include/arm-linux-androideabi", -- <asm/...>
		}
		buildoptions {
			"--target=armv7-none-linux-androideabi$(ANDROID_NDK_ABI)",
			"--gcc-toolchain=$(ANDROID_NDK_ROOT)/toolchains/llvm/prebuilt/linux-x86_64",
			"--sysroot=$(ANDROID_NDK_ROOT)/toolchains/llvm/prebuilt/linux-x86_64/sysroot",
			"-mthumb",
			"-march=armv7-a",
			"-mfloat-abi=softfp",
			"-mfpu=neon",
			"-Wunused-value",
			"-Wundef",
		}
		linkoptions {
			"--target=armv7-none-linux-androideabi$(ANDROID_NDK_ABI)",
			"--gcc-toolchain=$(ANDROID_NDK_ROOT)/toolchains/llvm/prebuilt/linux-x86_64",
			"--sysroot=$(ANDROID_NDK_ROOT)/toolchains/llvm/prebuilt/linux-x86_64/sysroot",
			"$(ANDROID_NDK_ROOT)/platforms/android-$(ANDROID_NDK_ABI)/arch-arm/usr/lib/crtbegin_so.o",
			"$(ANDROID_NDK_ROOT)/platforms/android-$(ANDROID_NDK_ABI)/arch-arm/usr/lib/crtend_so.o",
			"-march=armv7-a",
			"-Wl,--fix-cortex-a8",
		}

	configuration { "android-arm64" }
		targetdir (build_dir .. "android-arm64/bin")
		objdir (build_dir .. "android-arm64/obj")
		libdirs {
			"$(ANDROID_NDK_ROOT)/sources/cxx-stl/llvm-libc++/libs/arm64-v8a",
		}
		includedirs {
			"$(ANDROID_NDK_ROOT)/sysroot/usr/include/aarch64-linux-android",
		}
		buildoptions {
			"--target=aarch64-none-linux-androideabi$(ANDROID_NDK_ABI)",
			"--gcc-toolchain=$(ANDROID_NDK_ROOT)/toolchains/llvm/prebuilt/linux-x86_64",
			"--sysroot=$(ANDROID_NDK_ROOT)/toolchains/llvm/prebuilt/linux-x86_64/sysroot",
			"-march=armv8-a",
			"-Wunused-value",
			"-Wundef",
		}
		linkoptions {
			"--target=aarch64-none-linux-androideabi$(ANDROID_NDK_ABI)",
			"--gcc-toolchain=$(ANDROID_NDK_ROOT)/toolchains/llvm/prebuilt/linux-x86_64",
			"--sysroot=$(ANDROID_NDK_ROOT)/toolchains/llvm/prebuilt/linux-x86_64/sysroot",
			"$(ANDROID_NDK_ROOT)/platforms/android-$(ANDROID_NDK_ABI)/arch-arm64/usr/lib/crtbegin_so.o",
			"$(ANDROID_NDK_ROOT)/platforms/android-$(ANDROID_NDK_ABI)/arch-arm64/usr/lib/crtend_so.o",
			"-march=armv8-a",
		}

	configuration { "mingw-*" }
		defines { "WIN32" }
		includedirs { path.join(CROWN_DIR, "3rdparty/bx/include/compat/mingw") }
		buildoptions {
			"-fdata-sections",
			"-ffunction-sections",
			"-msse2",
			"-Wunused-value",
			"-Wundef",
		}
		buildoptions_cpp {
			"-std=c++14",
		}
		linkoptions {
			"-Wl,--gc-sections",
			"-static",
			"-static-libgcc",
			"-static-libstdc++",
		}

	configuration { "x32", "mingw-gcc" }
		targetdir (path.join(build_dir, "mingw32/bin"))
		objdir (path.join(build_dir, "mingw32/obj"))
		libdirs {
			lib_dir .. "../build/mingw32/bin",
		}
		buildoptions {
			"-m32",
			"-mstackrealign",
		}

	configuration { "x64", "mingw-gcc" }
		targetdir (path.join(build_dir, "mingw64/bin"))
		objdir (path.join(build_dir, "mingw64/obj"))
		libdirs {
			lib_dir .. "../build/mingw64/bin",
		}
		buildoptions { "-m64" }

	configuration { "vs*" }
		includedirs { CROWN_DIR .. "3rdparty/bx/include/compat/msvc" }
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
			"/Ob2", -- The Inline Function Expansion
			"/we4715", -- Not all control paths return a value
		}
		linkoptions {
			"/ignore:4199", -- LNK4199: /DELAYLOAD:*.dll ignored; no imports found from *.dll
			"/ignore:4221", -- LNK4221: This object file does not define any previously undefined public symbols, so it will not be used by any link operation that consumes this library
			"/ignore:4099", -- LNK4099: The linker was unable to find your .pdb file.
		}

	configuration { "x32", "vs*" }
		targetdir (build_dir .. "windows32" .. "/bin")
		objdir (build_dir .. "windows32" .. "/obj")

	configuration { "x64", "vs*" }
		defines {
			"_WIN64",
		}
		targetdir (build_dir .. "windows64" .. "/bin")
		objdir (build_dir .. "windows64" .. "/obj")

	configuration {} -- reset configuration
end

function strip()

	configuration { "android-arm", "release"}
		postbuildcommands {
			"$(SILENT) echo Stripping symbols",
			"$(SILENT) $(ANDROID_NDK_ROOT)/toolchains/llvm/prebuilt/linux-x86_64/bin/arm-linux-androideabi-strip -s \"$(TARGET)\""
		}

	configuration { "android-arm64", "release"}
		postbuildcommands {
			"$(SILENT) echo Stripping symbols",
			"$(SILENT) $(ANDROID_NDK_ROOT)/toolchains/llvm/prebuilt/linux-x86_64/bin/aarch64-linux-android-strip -s \"$(TARGET)\""
		}

	configuration { "linux-*", "release" }
		postbuildcommands {
			"$(SILENT) echo Stripping symbols",
			"$(SILENT) strip -s \"$(TARGET)\""
		}

	configuration { "mingw*", "Release" }
		postbuildcommands {
			"$(SILENT) echo Stripping symbols.",
			"$(SILENT) $(MINGW)/bin/strip -s \"$(TARGET)\""
		}

	configuration {} -- reset configuration
end
