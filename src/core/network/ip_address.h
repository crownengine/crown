/*
 * Copyright (c) 2012-2021 Daniele Bartolini et al.
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
	u32 address() const;
};

static const IPAddress IP_ADDRESS_LOOPBACK = { 127, 0, 0, 1 };

} // namespace crown
