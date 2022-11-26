/*
 * Copyright (c) 2012-2022 Daniele Bartolini et al.
 * License: https://github.com/crownengine/crown/blob/master/LICENSE
 */

#pragma once

#include "core/platform.h"

#define CROWN_VERSION_MAJOR 0
#define CROWN_VERSION_MINOR 47
#define CROWN_VERSION_PATCH 0

#define CROWN_VERSION CE_STRINGIZE(CROWN_VERSION_MAJOR) \
	"." CE_STRINGIZE(CROWN_VERSION_MINOR)               \
	"." CE_STRINGIZE(CROWN_VERSION_PATCH)

#ifndef CROWN_DEBUG
	#define CROWN_DEBUG 0
#endif

#ifndef CROWN_DEVELOPMENT
	#define CROWN_DEVELOPMENT 0
#endif

#define CROWN_CAN_COMPILE ((CROWN_DEBUG || CROWN_DEVELOPMENT) \
	&& (CROWN_PLATFORM_LINUX || CROWN_PLATFORM_WINDOWS)       \
	? 1 : 0)

#ifndef CROWN_TOOLS
	#define CROWN_TOOLS 0
#endif

#ifndef CROWN_BUILD_UNIT_TESTS
	#define CROWN_BUILD_UNIT_TESTS 1
#endif

#if !defined(CROWN_PHYSICS_BULLET) \
	&& !defined(CROWN_PHYSICS_NOOP)

	#ifndef CROWN_PHYSICS_BULLET
		#define CROWN_PHYSICS_BULLET 1
	#endif

	#ifndef CROWN_PHYSICS_NOOP
		#define CROWN_PHYSICS_NOOP 0
	#endif
#else
	#ifndef CROWN_PHYSICS_BULLET
		#define CROWN_PHYSICS_BULLET 0
	#endif

	#ifndef CROWN_PHYSICS_NOOP
		#define CROWN_PHYSICS_NOOP 0
	#endif
#endif // if !defined(CROWN_PHYSICS_BULLET) && !defined(CROWN_PHYSICS_NOOP)

#if !defined(CROWN_SOUND_OPENAL) \
	&& !defined(CROWN_SOUND_NOOP)

	#ifndef CROWN_SOUND_OPENAL
		#define CROWN_SOUND_OPENAL 1
	#endif

	#ifndef CROWN_SOUND_NOOP
		#define CROWN_SOUND_NOOP 0
	#endif
#else
	#ifndef CROWN_SOUND_OPENAL
		#define CROWN_SOUND_OPENAL 0
	#endif

	#ifndef CROWN_SOUND_NOOP
		#define CROWN_SOUND_NOOP 0
	#endif
#endif // if !defined(CROWN_SOUND_OPENAL) && !defined(CROWN_SOUND_NOOP)

#ifndef CROWN_DEFAULT_PIXELS_PER_METER
	#define CROWN_DEFAULT_PIXELS_PER_METER 32
#endif

#ifndef CROWN_DEFAULT_WINDOW_WIDTH
	#define CROWN_DEFAULT_WINDOW_WIDTH 1280
#endif

#ifndef CROWN_DEFAULT_WINDOW_HEIGHT
	#define CROWN_DEFAULT_WINDOW_HEIGHT 720
#endif

#ifndef CROWN_DEFAULT_CONSOLE_PORT
	#define CROWN_DEFAULT_CONSOLE_PORT 10001
#endif

#ifndef CROWN_DEFAULT_COMPILER_PORT
	#define CROWN_DEFAULT_COMPILER_PORT 10618
#endif

#ifndef CROWN_BOOT_CONFIG
	#define CROWN_BOOT_CONFIG "boot"
#endif

#ifndef CROWN_DATA_DIRECTORY
	#define CROWN_DATA_DIRECTORY "data"
#endif

#ifndef CROWN_TEMP_DIRECTORY
	#define CROWN_TEMP_DIRECTORY "temp"
#endif

#ifndef CROWN_DATAIGNORE
	#define CROWN_DATAIGNORE ".dataignore"
#endif

#ifndef CROWN_LAST_LOG
	#define CROWN_LAST_LOG "last.log"
#endif

#ifndef CROWN_MAX_JOYPADS
	#define CROWN_MAX_JOYPADS 4
#endif

#ifndef CROWN_LUA_MAX_VECTOR3_SIZE
	#define CROWN_LUA_MAX_VECTOR3_SIZE (129*1024)
#endif

#ifndef CROWN_LUA_MAX_QUATERNION_SIZE
	#define CROWN_LUA_MAX_QUATERNION_SIZE (128*1024)
#endif

#ifndef CROWN_LUA_MAX_MATRIX4X4_SIZE
	#define CROWN_LUA_MAX_MATRIX4X4_SIZE (128*1024)
#endif

#ifndef CROWN_MAX_OS_EVENTS
	#define CROWN_MAX_OS_EVENTS 128
#endif
