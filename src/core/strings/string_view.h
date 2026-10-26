/*
 * Copyright (c) 2012-2026 Daniele Bartolini et al.
 * SPDX-License-Identifier: MIT
 */

#pragma once

#include "core/strings/types.h"
#include "core/types.h"

namespace crown
{
/// String view.
///
/// @ingroup String
struct StringView
{
	u32 _length;
	const char *_data;

	///
	StringView();

	///
	explicit StringView(const char *str);

	///
	StringView(const char *str, u32 len);

	///
	StringView &operator=(const char *str);

	///
	u32 length() const;

	///
	const char *data() const;

	///
	StringId32 to_string_id() const;
};

} // namespace crown
