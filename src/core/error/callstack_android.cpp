/*
 * Copyright (c) 2012-2021 Daniele Bartolini et al.
 * License: https://github.com/dbartolini/crown/blob/master/LICENSE
 */

#include "core/platform.h"

#if CROWN_PLATFORM_ANDROID

#include "core/strings/string_stream.inl"

namespace crown
{
namespace error
{
	void callstack(StringStream& ss)
	{
		ss << "Not supported";
	}

} // namespace error

} // namespace crown

#endif // CROWN_PLATFORM_ANDROID
