/*
 * Copyright (c) 2012-2017 Daniele Bartolini and individual contributors.
 * License: https://github.com/taylor001/crown/blob/master/LICENSE
 */

#include "platform.h"

#if CROWN_PLATFORM_ANDROID

#include "string_stream.h"

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
