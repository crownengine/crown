/*
 * Copyright (c) 2012-2021 Daniele Bartolini et al.
 * License: https://github.com/dbartolini/crown/blob/master/LICENSE
 */

#pragma once

#include "core/strings/string_stream.h"
#include "core/strings/types.h"

namespace crown
{
namespace error
{
	/// Fills @a ss with the current call stack.
	void callstack(StringStream& ss);

} // namespace error

} // namespace crown
