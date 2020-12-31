/*
 * Copyright (c) 2012-2021 Daniele Bartolini et al.
 * License: https://github.com/dbartolini/crown/blob/master/LICENSE
 */

#pragma once

#include "core/network/ip_address.h"

namespace crown
{
inline u32 IPAddress::address() const
{
	return (u32(a) << 24) | (u32(b) << 16) | (u32(c) << 8) | u32(d);
}

} // namespace crown
