/*
 * Copyright (c) 2012-2025 Daniele Bartolini et al.
 * SPDX-License-Identifier: MIT
 */

#include "core/platform.h"

#if CROWN_PLATFORM_EMSCRIPTEN
#include "core/error/callstack.h"
#include "core/memory/allocator.h"
#include "core/memory/globals.h"
#include <emscripten/emscripten.h>

namespace crown
{
namespace error
{
	s32 callstack_init()
	{
		return 0;
	}

	void callstack_shutdown()
	{
		CE_NOOP();
	}

	void callstack(log_internal::System system, LogSeverity::Enum severity)
	{
		int size = emscripten_get_callstack(EM_LOG_C_STACK | EM_LOG_JS_STACK, NULL, 0);

		char *data = (char *)default_allocator().allocate(size);
		emscripten_get_callstack(EM_LOG_C_STACK | EM_LOG_JS_STACK, data, size);
		log_internal::logx(severity, system, data);
		default_allocator().deallocate(data);
	}

} // namespace error

} // namespace crown

#endif // if CROWN_PLATFORM_EMSCRIPTEN
