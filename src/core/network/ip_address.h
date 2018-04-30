/*
 * Copyright (c) 2012-2018 Daniele Bartolini and individual contributors.
 * License: https://github.com/dbartolini/crown/blob/master/LICENSE
 */

#pragma once

#include "core/types.h"

namespace crown
{
/// IP address.
///
/// @ingroup Network
struct IPAddress
{
	u8 a, b, c, d;

	/// Returns the IP address as packed 32-bit integer.
	u32 address() const
	{
		return (u32(a) << 24) | (u32(b) << 16) | (u32(c) << 8) | u32(d);
	}
};

const IPAddress IP_ADDRESS_LOOPBACK = { 127, 0, 0, 1 };

} // namespace crown
