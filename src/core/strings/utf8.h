/*
 * Copyright (c) 2012-2016 Daniele Bartolini and individual contributors.
 * License: https://github.com/taylor001/crown/blob/master/LICENSE
 */

#pragma once

#include "types.h"

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
	u32 decode(u32* state, u32* codep, u32 byte);

} // namespace utf8
} // namespace crown
