/*
 * Copyright (c) 2012-2021 Daniele Bartolini et al.
 * License: https://github.com/dbartolini/crown/blob/master/LICENSE
 */

// Adapted from Branimir Karadžić's platform.h (https://github.com/bkaradzic/bx)

#pragma once

#define CROWN_COMPILER_CLANG 0
#define CROWN_COMPILER_GCC 0
#define CROWN_COMPILER_MSVC 0

#define CROWN_PLATFORM_ANDROID 0
#define CROWN_PLATFORM_IOS 0
#define CROWN_PLATFORM_LINUX 0
#define CROWN_PLATFORM_OSX 0
#define CROWN_PLATFORM_WINDOWS 0

#define CROWN_CPU_ARM  0
#define CROWN_CPU_JIT  0
#define CROWN_CPU_MIPS 0
#define CROWN_CPU_PPC  0
#define CROWN_CPU_X86  0

#define CROWN_ARCH_32BIT 0
#define CROWN_ARCH_64BIT 0

#define CROWN_CPU_ENDIAN_BIG 0
#define CROWN_CPU_ENDIAN_LITTLE 0

// http://sourceforge.net/apps/mediawiki/predef/index.php?title=Compilers
#if defined(_MSC_VER)
	#undef CROWN_COMPILER_MSVC
	#define CROWN_COMPILER_MSVC 1
#elif defined(__clang__)
	// clang defines __GNUC__
	#undef CROWN_COMPILER_CLANG
	#define CROWN_COMPILER_CLANG 1
#elif defined(__GNUC__)
	#undef CROWN_COMPILER_GCC
	#define CROWN_COMPILER_GCC 1
#else
	#error "CROWN_COMPILER_* is not defined!"
#endif

// http://sourceforge.net/apps/mediawiki/predef/index.php?title=Operating_Systems
#if defined(_WIN32) || defined(_WIN64)
	#undef CROWN_PLATFORM_WINDOWS
// http://msdn.microsoft.com/en-us/library/6sehtctf.aspx
	#if !defined(WINVER) && !defined(_WIN32_WINNT)
// Windows 7 and above
		#define WINVER 0x0601
		#define _WIN32_WINNT 0x0601
	#endif // !defined(WINVER) && !defined(_WIN32_WINNT)
	#define CROWN_PLATFORM_WINDOWS 1
#elif defined(__ANDROID__)
// Android compiler defines __linux__
	#undef CROWN_PLATFORM_ANDROID
	#define CROWN_PLATFORM_ANDROID 1
#elif defined(__linux__)
	#undef CROWN_PLATFORM_LINUX
	#define CROWN_PLATFORM_LINUX 1
#elif defined(__ENVIRONMENT_IPHONE_OS_VERSION_MIN_REQUIRED__)
	#undef CROWN_PLATFORM_IOS
	#define CROWN_PLATFORM_IOS 1
#elif defined(__ENVIRONMENT_MAC_OS_X_VERSION_MIN_REQUIRED__)
	#undef CROWN_PLATFORM_OSX
	#define CROWN_PLATFORM_OSX 1
#else
#	error "CROWN_PLATFORM_* is not defined!"
#endif

#define CROWN_PLATFORM_POSIX (CROWN_PLATFORM_ANDROID \
						|| CROWN_PLATFORM_IOS \
						|| CROWN_PLATFORM_LINUX \
						|| CROWN_PLATFORM_OSX)

// http://sourceforge.net/apps/mediawiki/predef/index.php?title=Architectures
#if defined(__arm__)
	#undef CROWN_CPU_ARM
	#define CROWN_CPU_ARM 1
	#define CROWN_CACHE_LINE_SIZE 64
#elif defined(__MIPSEL__) || defined(__mips_isa_rev) // defined(mips)
	#undef CROWN_CPU_MIPS
	#define CROWN_CPU_MIPS 1
	#define CROWN_CACHE_LINE_SIZE 64
#elif defined(_M_PPC) || defined(__powerpc__) || defined(__powerpc64__)
	#undef CROWN_CPU_PPC
	#define CROWN_CPU_PPC 1
	#define CROWN_CACHE_LINE_SIZE 128
#elif defined(_M_IX86) || defined(_M_X64) || defined(__i386__) || defined(__x86_64__)
	#undef CROWN_CPU_X86
	#define CROWN_CPU_X86 1
	#define CROWN_CACHE_LINE_SIZE 64
#else // PNaCl doesn't have CPU defined.
	#undef CROWN_CPU_JIT
	#define CROWN_CPU_JIT 1
	#define CROWN_CACHE_LINE_SIZE 64
#endif //

#if defined(__x86_64__) || defined(_M_X64) || defined(__64BIT__) || defined(__powerpc64__) || defined(__ppc64__)
	#undef CROWN_ARCH_64BIT
	#define CROWN_ARCH_64BIT 64
#else
	#undef CROWN_ARCH_32BIT
	#define CROWN_ARCH_32BIT 32
#endif //

#if CROWN_CPU_PPC
	#undef CROWN_CPU_ENDIAN_BIG
	#define CROWN_CPU_ENDIAN_BIG 1
#else
	#undef CROWN_CPU_ENDIAN_LITTLE
	#define CROWN_CPU_ENDIAN_LITTLE 1
#endif

#if CROWN_COMPILER_GCC
	#define CROWN_COMPILER_NAME "GCC"
#elif CROWN_COMPILER_MSVC
	#define CROWN_COMPILER_NAME "MSVC"
#endif

#if CROWN_PLATFORM_ANDROID
	#define CROWN_PLATFORM_NAME "android"
#elif CROWN_PLATFORM_IOS
	#define CROWN_PLATFORM_NAME "ios"
#elif CROWN_PLATFORM_LINUX
	#define CROWN_PLATFORM_NAME "linux"
#elif CROWN_PLATFORM_OSX
	#define CROWN_PLATFORM_NAME "osx"
#elif CROWN_PLATFORM_WINDOWS
	#define CROWN_PLATFORM_NAME "windows"
#endif // CROWN_PLATFORM_

#if CROWN_CPU_ARM
	#define CROWN_CPU_NAME "ARM"
#elif CROWN_CPU_MIPS
	#define CROWN_CPU_NAME "MIPS"
#elif CROWN_CPU_PPC
	#define CROWN_CPU_NAME "PowerPC"
#elif CROWN_CPU_JIT
	#define CROWN_CPU_NAME "JIT-VM"
#elif CROWN_CPU_X86
	#define CROWN_CPU_NAME "x86"
#endif // CROWN_CPU_

#if CROWN_ARCH_32BIT
	#define CROWN_ARCH_NAME "32-bit"
#elif CROWN_ARCH_64BIT
	#define CROWN_ARCH_NAME "64-bit"
#endif // CROWN_ARCH_
