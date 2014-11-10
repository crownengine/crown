/*
 * Copyright (c) 2012-2014 Daniele Bartolini and individual contributors.
 * License: https://github.com/taylor001/crown/blob/master/LICENSE
 */

#pragma once

#include "string_utils.h"

namespace crown
{

/// @defgroup Path Path

/// Functions for operating on strings as file paths.
///
/// @ingroup Path
namespace path
{
	/// Returns whether the @a path is absolute.
	bool is_absolute_path(const char* path);

	/// Returns whether the @a path is the root path.
	bool is_root_path(const char* path);

	void pathname(const char* path, char* str, size_t len);
	void filename(const char* path, char* str, size_t len);
	void basename(const char* path, char* str, size_t len);
	void extension(const char* path, char* str, size_t len);
	void filename_without_extension(const char* path, char* str, size_t len);

	//bool segments(const char* path, Array<Str>& ret);
	void strip_trailing_separator(const char* path, char* ret, size_t len);
} // namespace path
} // namespace crown
