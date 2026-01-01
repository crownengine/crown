/*
 * Copyright (c) 2012-2026 Daniele Bartolini et al.
 * SPDX-License-Identifier: MIT
 */

#include "core/platform.h"

#if CROWN_PLATFORM_ANDROID
#include "core/debug/callstack.h"

namespace crown
{
namespace debug
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
		log_internal::logx(severity, system, "Callstack not supported on this platform.");
	}

} // namespace debug

} // namespace crown

#endif // if CROWN_PLATFORM_ANDROID
