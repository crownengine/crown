/*
 * Copyright (c) 2012-2016 Daniele Bartolini and individual contributors.
 * License: https://github.com/taylor001/crown/blob/master/LICENSE
 */

#pragma once

#include "platform.h"
#include "string_types.h"

namespace crown
{
#if CROWN_PLATFORM_POSIX
	const char PATH_SEPARATOR = '/';
#elif CROWN_PLATFORM_WINDOWS
	const char PATH_SEPARATOR = '\\';
#endif // CROWN_PLATFORM_POSIX

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
	void join(const char* path_a, const char* path_b, DynamicString& path);

	/// Returns the basename of the @a path.
	/// @note
	/// "/home/project/texture.tga" -> "texture.tga"
	/// "/home/project" -> "project"
	/// "/" -> ""
	const char* basename(const char* path);

	/// Returns the extension of the @a path or NULL.
	/// @note
	/// "/home/texture.tga" -> "tga"
	/// "/home/texture" -> NULL
	const char* extension(const char* path);
} // namespace path
} // namespace crown
