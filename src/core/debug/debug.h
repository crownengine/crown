/*
 * Copyright (c) 2012-2026 Daniele Bartolini et al.
 * SPDX-License-Identifier: MIT
 */

#pragma once

#include "core/types.h"

/// @defgroup Debug Debug
/// @ingroup Core
namespace crown
{
struct CrashType
{
	enum Enum
	{
		DIVISION_BY_ZERO,
		UNALIGNED_ACCESS,
		SEGMENTATION_FAULT,
		OUT_OF_MEMORY,
		ASSERT,

		COUNT
	};
};

/// Debug utils.
///
/// @ingroup Debug
namespace debug
{
	/// Triggers a breakpoint.
	void breakpoint();

	/// Crashes the application.
	void crash(CrashType::Enum type);

} // namespace debug

} // namespace crown
