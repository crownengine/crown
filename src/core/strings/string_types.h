/*
 * Copyright (c) 2012-2016 Daniele Bartolini and individual contributors.
 * License: https://github.com/taylor001/crown/blob/master/LICENSE
 */

#pragma once

#include "container_types.h"

/// @defgroup String String
namespace crown
{
struct DynamicString;
struct FixedString;
struct StringId32;
struct StringId64;

/// Stream of characters.
///
/// @ingroup String
typedef Array<char> StringStream;

} // namespace crown
