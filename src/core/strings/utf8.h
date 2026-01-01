/*
 * Copyright (c) 2012-2026 Daniele Bartolini et al.
 * SPDX-License-Identifier: MIT
 */

#pragma once

#include "core/types.h"

#define UTF8_ACCEPT 0
#define UTF8_REJECT 1

namespace crown
{
/// Functions to manipulate UTF8-encoded data.
///
/// @ingroup String
namespace utf8
{
	/// See http://bjoern.hoehrmann.de/utf-8/decoder/dfa/ for details.
	u32 decode(u32 *state, u32 *codep, u32 byte);

} // namespace utf8

} // namespace crown
