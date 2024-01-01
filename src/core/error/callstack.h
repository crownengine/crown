/*
 * Copyright (c) 2012-2024 Daniele Bartolini et al.
 * SPDX-License-Identifier: MIT
 */

#pragma once

#include "core/strings/string_stream.h"
#include "core/strings/types.h"

namespace crown
{
namespace error
{
	/// Fills @a ss with the current call stack.
	void callstack(StringStream &ss);

} // namespace error

} // namespace crown
