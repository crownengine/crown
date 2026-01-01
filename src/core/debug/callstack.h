/*
 * Copyright (c) 2012-2026 Daniele Bartolini et al.
 * SPDX-License-Identifier: MIT
 */

#pragma once

#include "core/types.h"
#include "device/log.h"

namespace crown
{
namespace debug
{
	/// Initializes the callstack subsystem.
	s32 callstack_init();

	/// Shutdowns the callstack subsystem.
	void callstack_shutdown();

	/// Logs the current call stack.
	void callstack(log_internal::System system, LogSeverity::Enum severity = LogSeverity::LOG_INFO);

} // namespace debug

} // namespace crown
