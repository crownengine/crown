/*
 * Copyright (c) 2012-2023 Daniele Bartolini et al.
 * SPDX-License-Identifier: MIT
 */

#include "core/platform.h"

#if CROWN_PLATFORM_EMSCRIPTEN
#include "core/memory/globals.h"
#include "core/strings/string_stream.inl"
#include <emscripten/emscripten.h>

namespace crown
{
namespace error
{
	void callstack(StringStream &ss)
	{
		int size = emscripten_get_callstack(EM_LOG_C_STACK | EM_LOG_JS_STACK, NULL, 0);

		char *data = (char *)default_allocator().allocate(size);
		emscripten_get_callstack(EM_LOG_C_STACK | EM_LOG_JS_STACK, data, size);

		ss << data;

		default_allocator().deallocate(data);
	}

} // namespace error

} // namespace crown

#endif // if CROWN_PLATFORM_EMSCRIPTEN
