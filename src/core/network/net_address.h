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

	NetAddress(uint8_t a, uint8_t b, uint8_t c, uint8_t d)
		: _addr(0)
	{
		set(a, b, c, d);
	}

	/// Returns the IP address as packed 32-bit integer.
	uint32_t address() const
	{
		return _addr;
	}

	void set(uint8_t a, uint8_t b, uint8_t c, uint8_t d)
	{
		_addr = 0;
		_addr |= uint32_t(a) << 24;
		_addr |= uint32_t(b) << 16;
		_addr |= uint32_t(c) << 8;
		_addr |= uint32_t(d) << 0;
	}

	uint32_t _addr;
};

} // namespace crown
