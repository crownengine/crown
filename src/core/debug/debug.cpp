/*
 * Copyright (c) 2012-2026 Daniele Bartolini et al.
 * SPDX-License-Identifier: MIT
 */

#include "core/debug/debug.h"
#include "core/error/error.inl"
#include "core/memory/allocator.h"
#include "core/memory/globals.h"
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

	void crash(CrashType::Enum type)
	{
		CE_ALIGN_DECL(alignof(s64), u8 buf[16]);
		volatile s64 zero = 0;

		switch (type) {
		case CrashType::DIVISION_BY_ZERO:
			*((f32 *)buf) = 1.0f / f32(zero);
			break;

		case CrashType::UNALIGNED_ACCESS:
			*((s64 *)&buf[1]) = zero;
			break;

		case CrashType::SEGMENTATION_FAULT:
			*((s64 *)(uintptr_t)0xffffffffu) = zero;
			break;

		case CrashType::OUT_OF_MEMORY: {
			s64 size = 512*1024*1024;
			for (s64 i = 0; i < INT64_MAX; ++i, size *= 2)
				*((s64 *)default_allocator().allocate(size)) = zero;
			break;
		}

		case CrashType::ASSERT:
			CE_ASSERT(false, "False");
			break;

		default:
			break;
		}
	}

} // namespace debug

} // namespace crown
