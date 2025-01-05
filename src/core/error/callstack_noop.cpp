/*
 * Copyright (c) 2012-2025 Daniele Bartolini et al.
 * SPDX-License-Identifier: MIT
 */

#include "core/platform.h"

#if CROWN_PLATFORM_ANDROID
#include "core/strings/string_stream.inl"

namespace crown
{
namespace error
{
	void callstack(StringStream &ss)
	{
		ss << "Not supported";
	}

} // namespace error

} // namespace crown

#endif
