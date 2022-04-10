/*
 * Copyright (c) 2012-2022 Daniele Bartolini et al.
 * License: https://github.com/dbartolini/crown/blob/master/LICENSE
 */

#pragma once

#include "core/types.h"

namespace crown
{
///
const char* skip_block(const char* str, char a, char b);

///
int wildcmp(const char *wild, const char *str);

} // namespace crown
