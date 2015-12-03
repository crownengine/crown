/*
 * Copyright (c) 2012-2015 Daniele Bartolini and individual contributors.
 * License: https://github.com/taylor001/crown/blob/master/LICENSE
 */

// Adapted from Branimir Karadžić's config.h (https://github.com/bkaradzic/bx)

#pragma once

#include "platform.h"

#ifndef CROWN_DEBUG
	#define CROWN_DEBUG 0
#endif // CROWN_DEBUG

#if !defined(CROWN_SOUND_OPENAL) \
	&& !defined(CROWN_SOUND_NULL)

	#ifndef CROWN_SOUND_OPENAL
		#define CROWN_SOUND_OPENAL (CROWN_PLATFORM_LINUX || CROWN_PLATFORM_WINDOWS)
	#endif // CROWN_SOUND_OPENAL

	#ifndef CROWN_SOUND_NULL
		#define CROWN_SOUND_NULL (!CROWN_SOUND_OPENAL || CROWN_PLATFORM_ANDROID)
	#endif // CROWN_SOUND_NULL
#else
	#ifndef CROWN_SOUND_OPENAL
		#define CROWN_SOUND_OPENAL 0
	#endif

	#ifndef CROWN_SOUND_NULL
		#define CROWN_SOUND_NULL 0
	#endif
#endif

#ifndef CROWN_DEFAULT_PIXELS_PER_METER
	#define CROWN_DEFAULT_PIXELS_PER_METER 32
#endif // CROWN_DEFAULT_PIXELS_PER_METER

#ifndef CROWN_DEFAULT_WINDOW_WIDTH
	#define CROWN_DEFAULT_WINDOW_WIDTH 1280
#endif // CROWN_DEFAULT_WINDOW_WIDTH

#ifndef CROWN_DEFAULT_WINDOW_HEIGHT
	#define CROWN_DEFAULT_WINDOW_HEIGHT 720
#endif // CROWN_DEFAULT_WINDOW_HEIGHT

#ifndef CROWN_DEFAULT_CONSOLE_PORT
	#define CROWN_DEFAULT_CONSOLE_PORT 10001
#endif // CROWN_DEFAULT_CONSOLE_PORT

#ifndef CROWN_DATA_DIRECTORY
	#define CROWN_DATA_DIRECTORY "data"
#endif // CROWN_DATA_DIRECTORY

#ifndef CROWN_MAX_JOYPADS
	#define CROWN_MAX_JOYPADS 4
#endif // CROWN_MAX_JOYPADS

#ifndef CE_MAX_UNITS
	#define CE_MAX_UNITS 65000 // Per world
#endif // CE_MAX_UNITS

#ifndef CE_MAX_CAMERAS
	#define CE_MAX_CAMERAS 16 // Per world
#endif // CE_MAX_CAMERAS

#ifndef CE_MAX_ACTORS
	#define CE_MAX_ACTORS 1024 // Per world
#endif // CE_MAX_ACTORS

#ifndef CE_MAX_CONTROLLERS
	#define CE_MAX_CONTROLLERS 16 // Per world
#endif // CE_MAX

#ifndef CE_MAX_TRIGGERS
	#define CE_MAX_TRIGGERS 1024 // Per world
#endif // CE_MAX

#ifndef CE_MAX_JOINTS
	#define CE_MAX_JOINTS 512 // Per world
#endif // CE_MAX

#ifndef CE_MAX_SOUND_INSTANCES
	#define CE_MAX_SOUND_INSTANCES 64 // Per world
#endif // CE_MAX

#ifndef CE_MAX_RAYCASTS
	#define CE_MAX_RAYCASTS 8 // Per World
#endif // CE_MAX

#ifndef CE_MAX_RAY_INTERSECTIONS
	#define CE_MAX_RAY_INTERSECTIONS 16
#endif // CE_MAX

#ifndef CE_MAX_CAMERA_COMPONENTS
	#define CE_MAX_CAMERA_COMPONENTS 16 // Per unit
#endif // CE_MAX

#ifndef CE_MAX_SPRITE_COMPONENTS
	#define CE_MAX_SPRITE_COMPONENTS 16 // Per unit
#endif // CE_MAX

#ifndef CE_MAX_ACTOR_COMPONENTS
	#define CE_MAX_ACTOR_COMPONENTS 16 // Per unit
#endif // CE_MAX

#ifndef CE_MAX_MATERIAL_COMPONENTS
	#define CE_MAX_MATERIAL_COMPONENTS 16 // Per unit
#endif // CE_MAX

#ifndef CROWN_MAX_DEBUG_LINES
	#define CROWN_MAX_DEBUG_LINES 32768 // Per DebugLine
#endif // CROWN_MAX_DEBUG_LINES

#ifndef CROWN_MAX_LUA_VECTOR3
	#define CROWN_MAX_LUA_VECTOR3 4096
#endif // CE_MAX

#ifndef CROWN_MAX_LUA_MATRIX4X4
	#define CROWN_MAX_LUA_MATRIX4X4 4096
#endif // CE_MAX

#ifndef CROWN_MAX_LUA_QUATERNION
	#define CROWN_MAX_LUA_QUATERNION 4096
#endif // CE_MAX
