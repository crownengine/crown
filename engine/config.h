/*
Copyright (c) 2013 Daniele Bartolini, Michele Rossi
Copyright (c) 2012 Daniele Bartolini, Simone Boscaratto

Permission is hereby granted, free of charge, to any person
obtaining a copy of this software and associated documentation
files (the "Software"), to deal in the Software without
restriction, including without limitation the rights to use,
copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the
Software is furnished to do so, subject to the following
conditions:

The above copyright notice and this permission notice shall be
included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
OTHER DEALINGS IN THE SOFTWARE.
*/

// Adapted from Branimir Karadžić's platform.h (https://github.com/bkaradzic/bx)

#pragma once

#define CROWN_VERSION_MAJOR "0"
#define CROWN_VERSION_MINOR "1"
#define CROWN_VERSION_MICRO "13"

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
// Windows Server 2003 with SP1, Windows XP with SP2 and above
		#define WINVER 0x0502
		#define _WIN32_WINNT 0x0502
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
	#define CROWN_PLATFORM_NAME "Android"
#elif CROWN_PLATFORM_IOS
	#define CROWN_PLATFORM_NAME "iOS"
#elif CROWN_PLATFORM_LINUX
	#define CROWN_PLATFORM_NAME "Linux"
#elif CROWN_PLATFORM_OSX
	#define CROWN_PLATFORM_NAME "OSX"
#elif CROWN_PLATFORM_WINDOWS
	#define CROWN_PLATFORM_NAME "Windows"
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

#define CE_PIXELS_PER_METER					32

#define CE_MAX_WORLDS						1024
#define CE_MAX_UNITS						65000				// Per world
#define CE_MAX_CAMERAS						16					// Per world
#define CE_MAX_ACTORS						1024				// Per world
#define CE_MAX_CONTROLLERS					16					// Per world
#define CE_MAX_TRIGGERS						1024				// Per world
#define CE_MAX_JOINTS						512					// Per world
#define CE_MAX_SOUND_INSTANCES				64					// Per world
#define CE_MAX_RAYCASTS						8					// Per World
#define CE_MAX_RAY_INTERSECTIONS			16

#define CE_MAX_CAMERA_COMPONENTS			16					// Per unit
#define CE_MAX_MESH_COMPONENTS				16					// Per unit
#define CE_MAX_SPRITE_COMPONENTS			16					// Per unit
#define CE_MAX_ACTOR_COMPONENTS				16					// Per unit
#define CE_MAX_MATERIAL_COMPONENTS			16					// Per unit

#define CE_MAX_CONSOLE_CLIENTS				32

#define CE_MAX_GUI_RECTS 					64					// Per Gui
#define CE_MAX_GUI_TRIANGLES 				64					// Per Gui
#define CE_MAX_GUI_IMAGES 					64					// Per Gui
#define CE_MAX_GUI_TEXTS 					64					// Per Gui

#define CE_MAX_DEBUG_LINES					2 * 1024			// Per DebugLine

#define CE_MAX_LUA_VECTOR2					4096
#define CE_MAX_LUA_VECTOR3					4096
#define CE_MAX_LUA_MATRIX4X4				4096
#define CE_MAX_LUA_QUATERNION				4096

#define CROWN_DEFAULT_WINDOW_WIDTH			1280
#define CROWN_DEFAULT_WINDOW_HEIGHT			720
