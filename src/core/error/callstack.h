/*
 * Copyright (c) 2012-2025 Daniele Bartolini et al.
 * SPDX-License-Identifier: MIT
 */

#pragma once

#include "core/types.h"
#include "device/log.h"

namespace crown
{
namespace error
{
	/// Logs the current call stack.
	void callstack(log_internal::System system, LogSeverity::Enum severity = LogSeverity::LOG_INFO);

} // namespace error

} // namespace crown
