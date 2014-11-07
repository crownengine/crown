/*
 * Copyright (c) 2012-2014 Daniele Bartolini and individual contributors.
 * License: https://github.com/taylor001/crown/blob/master/LICENSE
 */

#pragma once

#ifndef WIN32_LEAN_AND_MEAN
	#define WIN32_LEAN_AND_MEAN
#endif

#include <windows.h>

#undef NEAR
#undef FAR
#undef near
#undef far
#undef NO_ERROR
#undef ERROR
#undef MK_SHIFT
#undef MK_ALT
#undef min
#undef max
#undef rad1

#define __va_copy(dest, src) (dest = src)
