/*
 * Copyright (c) 2012-2026 Daniele Bartolini et al.
 * SPDX-License-Identifier: MIT
 */

#pragma once

#include "core/strings/types.h"
#include "core/types.h"

namespace crown
{
/// Returns the first occurrence of @a c in @a str, or NULL.
const char *find(const StringView &str, char c);

/// Returns the last occurrence of @a c in @a str, or NULL.
const char *find_reverse(const StringView &str, char c);

///
const char *skip_block(const char *str, char a, char b);

///
int wildcmp(const StringView &wild, const StringView &str);

///
int wildcmp(const char *wild, const char *str);

///
int strncasecmp(const char *str1, const char *str2, u32 len);

/// Returns 0 if success.
int from_hex(s64 &val, const char *hex);

} // namespace crown
