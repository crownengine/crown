/*
 * Copyright (c) 2012-2026 Daniele Bartolini et al.
 * SPDX-License-Identifier: MIT
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
