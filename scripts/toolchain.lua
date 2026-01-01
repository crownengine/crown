--
-- Copyright (c) 2012-2026 Daniele Bartolini et al.
-- SPDX-License-Identifier: MIT
--
-- Based on 3rdparty/bx/scripts/toolchain.lua by Branimir Karadzic.

local android = {};

local function androidToolchainRoot()
	if android.toolchainRoot == nil then
		local hostTags = {
			windows = "windows-x86_64",
			linux   = "linux-x86_64",
			macosx  = "darwin-x86_64"
		}

		android.toolchainRoot = "$(ANDROID_NDK_ROOT)/toolchains/llvm/prebuilt/" .. hostTags[os.get()]
	end

	return android.toolchainRoot;
end

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
			{ "wasm",          "emscripten/wasm"        },
		}
	}

	newoption {
		trigger     = "with-32bit-compiler",
		description = "Use 32-bit compiler instead 64-bit.",
	}

	if (_ACTION == nil) then return end

	if _ACTION == "clean" then
		os.rmdir(BUILD_DIR)
	end

	local compiler32bit = false
	if _OPTIONS["with-32bit-compiler"] then
		compiler32bit = true
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

			premake.gcc.cc  = androidToolchainRoot() .. "/bin/clang"
			premake.gcc.cxx = androidToolchainRoot() .. "/bin/clang++"
			premake.gcc.ar  = androidToolchainRoot() .. "/bin/llvm-ar"

			premake.gcc.llvm = true
			location(build_dir .. "projects/android-arm")

		elseif "android-arm64" == _OPTIONS["compiler"] then

			if not os.getenv("ANDROID_NDK_ABI") then
				print("Set ANDROID_NDK_ABI environment variable.")
			end

			if not os.getenv("ANDROID_NDK_ROOT") then
				print("Set ANDROID_NDK_ROOT environment variable.")
			end

			premake.gcc.cc  = androidToolchainRoot() .. "/bin/clang"
			premake.gcc.cxx = androidToolchainRoot() .. "/bin/clang++"
			premake.gcc.ar  = androidToolchainRoot() .. "/bin/llvm-ar"

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

			local mingwToolchain = "x86_64-w64-mingw32"
			if compiler32bit then
				mingwToolchain = "i686-w64-mingw32"
			end

			premake.gcc.cc  = "$(MINGW)/bin/" .. mingwToolchain .. "-gcc"
			premake.gcc.cxx = "$(MINGW)/bin/" .. mingwToolchain .. "-g++"
			premake.gcc.ar  = "$(MINGW)/bin/ar"
			premake.valac.cc  = premake.gcc.cc
			location(build_dir .. "projects/mingw")

		elseif "wasm" == _OPTIONS["compiler"] then

			if not os.getenv("EMSCRIPTEN") then
				print("Set EMSCRIPTEN environment variable.")
				os.exit(1)
			end

			premake.gcc.cc  = "$(EMSCRIPTEN)/emcc"
			premake.gcc.cxx = "$(EMSCRIPTEN)/em++"
			premake.gcc.ar  = "$(EMSCRIPTEN)/emar"

			premake.gcc.llvm = true
			location(build_dir .. "projects/wasm")
	end
	elseif _ACTION == "vs2019"
		or _ACTION == "vs2022"
		then

		if not os.is("windows") then
			print("Action not valid in current OS.")
		end

		local winKitVer = os.getenv("WindowsSDKVersion")
		if not winKitVer then
			print("Run vcvarsall.bat as part of your Visual Studio installation to set the environment variable 'WindowsSDKVersion'.")
			os.exit(1)
		end
		winKitVer = string.gsub(winKitVer, "\\", "")
		local action = premake.action.current()
		action.vstudio.windowsTargetPlatformVersion    = winKitVer
		action.vstudio.windowsTargetPlatformMinVersion = winKitVer

		location(build_dir .. "projects/" .. _ACTION)
	end

	flags {
		"Cpp20",
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

	configuration { "debug or development", "wasm" }
		linkoptions {
			"-gsource-map" -- See: https://emscripten.org/docs/porting/exceptions.html?highlight=gsource%20map#stack-traces
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
		links {
			"c",
			"dl",
			"m",
			"android",
			"log",
			"c++_shared",
		}
		buildoptions {
			"--gcc-toolchain=" .. androidToolchainRoot(),
			"--sysroot=" .. androidToolchainRoot() .. "/sysroot",
			--"-DANDROID",
			"-fPIC",
			"-no-canonical-prefixes",
			"-Wa,--noexecstack",
			"-fstack-protector-strong",
			"-ffunction-sections",
			"-Wunused-value",
			"-Wundef",
		}
		linkoptions {
			"--gcc-toolchain=" .. androidToolchainRoot(),
			"--sysroot=" .. androidToolchainRoot() .. "/sysroot",
			"-no-canonical-prefixes",
			"-Wl,--no-undefined",
			"-Wl,-z,noexecstack",
			"-Wl,-z,relro",
			"-Wl,-z,now",
		}

	configuration { "android-arm" }
		targetdir (build_dir .. "android-arm/bin")
		objdir (build_dir .. "android-arm/obj")
		buildoptions {
			"--target=armv7-none-linux-android" .. "$(ANDROID_NDK_ABI)",
			"-mthumb",
			"-march=armv7-a",
			"-mfloat-abi=softfp",
			"-mfpu=neon",
		}
		linkoptions {
			"--target=armv7-none-linux-android" .. "$(ANDROID_NDK_ABI)",
			"-march=armv7-a",
		}

	configuration { "android-arm64" }
		targetdir (build_dir .. "android-arm64/bin")
		objdir (build_dir .. "android-arm64/obj")
		buildoptions {
			"--target=aarch64-none-linux-android" .. "$(ANDROID_NDK_ABI)",
		}
		linkoptions {
			"--target=aarch64-none-linux-android" .. "$(ANDROID_NDK_ABI)",
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

	configuration { "wasm" }
		targetdir (path.join(build_dir, "wasm/bin"))
		objdir (path.join(build_dir, "wasm/obj"))
		libdirs { path.join(lib_dir, "lib/wasm") }
		buildoptions {
			"-Wunused-value",
			"-Wundef",
			"-pthread",
		}

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
			"/Zc:__cplusplus", -- https://learn.microsoft.com/en-us/cpp/build/reference/zc-cplusplus?view=msvc-170
			"/Zc:preprocessor", -- Enable preprocessor conformance mode.
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

	configuration { "android-*", "release"}
		postbuildcommands {
			"$(SILENT) echo Stripping symbols",
			"$(SILENT) " .. androidToolchainRoot() .. "/bin/llvm-strip -s \"$(TARGET)\""
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
