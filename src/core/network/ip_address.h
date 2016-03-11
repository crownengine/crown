/*
 * Copyright (c) 2012-2016 Daniele Bartolini and individual contributors.
 * License: https://github.com/taylor001/crown/blob/master/LICENSE
 */

#pragma once

#include "types.h"

namespace crown
{
/// IP address.
///
/// @ingroup Network
struct IPAddress
{
	u32 _addr;

	/// Initializes the address to 127.0.0.1
	IPAddress()
	{
		set(127, 0, 0, 1);
	}

	/// Initializes the address from individual components.
	IPAddress(u8 a, u8 b, u8 c, u8 d)
	{
		set(a, b, c, d);
	}

	/// Returns the IP address as packed 32-bit integer.
	u32 address() const
	{
		return _addr;
	}

	/// Sets the address from individual components.
	void set(u8 a, u8 b, u8 c, u8 d)
	{
		_addr = 0;
		_addr |= u32(a) << 24;
		_addr |= u32(b) << 16;
		_addr |= u32(c) << 8;
		_addr |= u32(d) << 0;
	}
};

} // namespace crown
