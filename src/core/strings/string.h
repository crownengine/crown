/*
 * Copyright (c) 2012-2026 Daniele Bartolini et al.
 * SPDX-License-Identifier: MIT
 */

#pragma once

#include "core/types.h"

namespace crown
{
///
const char *skip_block(const char *str, char a, char b);

///
int wildcmp(const char *wild, const char *str);

} // namespace crown
