/*
 * Copyright (c) 2012-2025 Daniele Bartolini et al.
 * SPDX-License-Identifier: MIT
 */

#include "core/platform.h"

#if CROWN_PLATFORM_ANDROID
#include "core/error/callstack.h"

namespace crown
{
namespace error
{
	void callstack(log_internal::System system, LogSeverity::Enum severity)
	{
		log_internal::logx(severity, system, "Not supported");
	}

} // namespace error

} // namespace crown

#endif
