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
	/// Returns whether the path segment @a segment is valid.
	/// @note
	/// The rules for valid segments are as follows:
	/// a) The empty string is not valid.
	/// b) Any string containing the slash character ('/') is not valid.
	/// c) Common notations for current ('.') and parent ('..') directory are forbidden.
	/// d) Any string containing segment or device separator characters on the local file system,
	/// such as the backslash ('\') and colon (':') on some file systems are not valid.
	/// (Thanks org.eclipse.core.runtime for the documentation ;D).
	bool is_valid_segment(const char* segment);

	/// Returns whether @a path is valid.
	bool is_valid_path(const char* path);

	/// Returns whether the path @a path is absolute.
	bool is_absolute_path(const char* path);

	void pathname(const char* path, char* str, size_t len);
	void filename(const char* path, char* str, size_t len);
	void basename(const char* path, char* str, size_t len);
	void extension(const char* path, char* str, size_t len);
	void filename_without_extension(const char* path, char* str, size_t len);

	//bool segments(const char* path, Array<Str>& ret);
	void strip_trailing_separator(const char* path, char* ret, size_t len);
} // namespace path
} // namespace crown
