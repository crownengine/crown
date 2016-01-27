/*
 * Copyright (c) 2012-2016 Daniele Bartolini and individual contributors.
 * License: https://github.com/taylor001/crown/blob/master/LICENSE
 */

#pragma once

#include "types.h"

namespace crown
{
/// Netwotk address helper
///
/// @ingroup Network
struct NetAddress
{
	/// Initializes the address to 127.0.0.1
	NetAddress()
		: _addr(0)
	{
		set(127, 0, 0, 1);
	}

	NetAddress(u8 a, u8 b, u8 c, u8 d)
		: _addr(0)
	{
		set(a, b, c, d);
	}

	/// Returns the IP address as packed 32-bit integer.
	u32 address() const
	{
		return _addr;
	}

	void set(u8 a, u8 b, u8 c, u8 d)
	{
		_addr = 0;
		_addr |= u32(a) << 24;
		_addr |= u32(b) << 16;
		_addr |= u32(c) << 8;
		_addr |= u32(d) << 0;
	}

	u32 _addr;
};

} // namespace crown
