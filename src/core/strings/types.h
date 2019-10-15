/*
 * Copyright (c) 2012-2018 Daniele Bartolini and individual contributors.
 * License: https://github.com/dbartolini/crown/blob/master/LICENSE
 */

#pragma once

#include "core/containers/types.h"

/// @defgroup String String
/// @ingroup Core
namespace crown
{
struct DynamicString;
struct StringView;
struct StringId32;
struct StringId64;
struct Guid;

typedef StringId64 ResourceId;

/// Stream of characters.
///
/// @ingroup String
typedef Array<char> StringStream;

} // namespace crown
