/*
 * Copyright (c) 2012-2021 Daniele Bartolini et al.
 * License: https://github.com/dbartolini/crown/blob/master/LICENSE
 */

#pragma once

#include "core/strings/types.h"

namespace crown
{
/// Functions for operating on strings as file paths.
///
/// @ingroup Filesystem
namespace path
{
	/// Returns whether the @a path is absolute.
	bool is_absolute(const char* path);

	/// Returns whether the @a path is relative.
	bool is_relative(const char* path);

	/// Returns whether the @a path is the root path.
	bool is_root(const char* path);

	/// Appends @a path_b to @a path_a and fills @a path with the result.
	void join(DynamicString& path, const char* path_a, const char* path_b);

	/// Returns the basename of the @a path.
	/// @note
	/// "/home/project/texture.tga" -> "texture.tga"
	/// "/home/project" -> "project"
	/// "/" -> ""
	const char* basename(const char* path);

	/// Returns the extension of the @a path or NULL.
	/// @note
	/// /home/texture.tga -> "tga"
	/// /home/texture -> NULL
	/// /home/.tga -> NULL
	const char* extension(const char* path);

	/// Returns whether the @a path has a trailing separator.
	bool has_trailing_separator(const char* path);

	/// Removes unnecessary dots and separators from @a path.
	void reduce(DynamicString& clean, const char* path);

} // namespace path

} // namespace crown
