/*
 * Copyright (c) 2012-2021 Daniele Bartolini et al.
 * License: https://github.com/dbartolini/crown/blob/master/LICENSE
 */

#pragma once

#include "core/containers/types.h"

namespace crown
{
/// Stream of characters.
///
/// @ingroup String
typedef Array<char> StringStream;

/// Functions to manipulate StringStream.
///
/// @ingroup String
namespace string_stream
{
	/// Returns the stream as a NUL-terminated string.
	const char* c_str(StringStream& s);

	///
	template <typename T> StringStream& stream_printf(StringStream& s, const char* format, T& val);

} // namespace string_stream

} // namespace crown
