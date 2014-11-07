/*
 * Copyright (c) 2012-2014 Daniele Bartolini and individual contributors.
 * License: https://github.com/taylor001/crown/blob/master/LICENSE
 */

#pragma once

namespace crown
{
namespace error
{
	/// Aborts the program execution logging an error message and the stacktrace if
	/// the platform supports it.
	void abort(const char* file, int line, const char* message, ...);
} // namespace error
} // namespace crown
