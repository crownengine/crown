/*
 * Copyright (c) 2012-2022 Daniele Bartolini et al.
 * License: https://github.com/crownengine/crown/blob/master/LICENSE
 */

#pragma once

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
};

} // namespace crown
