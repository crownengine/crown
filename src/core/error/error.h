/*
 * Copyright (c) 2012-2021 Daniele Bartolini et al.
 * License: https://github.com/dbartolini/crown/blob/master/LICENSE
 */

#pragma once

#include "core/types.h"

/// @defgroup Error Error
/// @ingroup Core
namespace crown
{
/// Error management.
///
/// @ingroup Error
namespace error
{
	/// Aborts the program execution logging an error message and the stacktrace if
	/// the platform supports it.
	void abort(const char* format, ...);

} // namespace error

} // namespace crown
