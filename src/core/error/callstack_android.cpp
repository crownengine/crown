/*
 * Copyright (c) 2012-2016 Daniele Bartolini and individual contributors.
 * License: https://github.com/taylor001/crown/blob/master/LICENSE
 */

#include "config.h"

#if CROWN_PLATFORM_ANDROID

#include "log.h"

namespace crown
{
namespace error
{
	void print_callstack()
	{
		loge("\tNot supported.");
	}
} // namespace error

} // namespace crown

#endif // CROWN_PLATFORM_ANDROID
