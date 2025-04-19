/*
 * Copyright (c) 2012-2025 Daniele Bartolini et al.
 * SPDX-License-Identifier: MIT
 */

#include "core/debug/debug.h"
#if CROWN_PLATFORM_EMSCRIPTEN
#   include <emscripten/emscripten.h>
#elif CROWN_PLATFORM_LINUX
#   include <signal.h>
#endif

namespace crown
{
namespace debug
{
	void breakpoint()
	{
#if CROWN_COMPILER_MSVC
		__debugbreak();
#elif CROWN_PLATFORM_EMSCRIPTEN
		emscripten_debugger();
#elif CROWN_PLATFORM_LINUX
		raise(SIGTRAP);
#elif CROWN_COMPILER_GCC || CROWN_COMPILER_CLANG
		__builtin_trap();
#else
#   error "Unavailable"
#endif
	}

} // namespace debug

} // namespace crown
