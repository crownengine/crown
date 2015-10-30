/*
 * Copyright (c) 2012-2015 Daniele Bartolini and individual contributors.
 * License: https://github.com/taylor001/crown/blob/master/LICENSE
 */

#include "path.h"
#include "platform.h"
#include "string_utils.h"
#include <ctype.h> // isalpha

namespace crown
{
namespace path
{
	bool is_absolute_path(const char* path)
	{
		CE_ASSERT(path != NULL, "Path must be != NULL");
#if CROWN_PLATFORM_POSIX
		return strlen(path) > 0 && path[0] == SEPARATOR;
#elif CROWN_PLATFORM_WINDOWS
		return strlen(path) > 2 && isalpha(path[0]) && path[1] == ':' && path[2] == SEPARATOR;
#endif
	}

	bool is_root_path(const char* path)
	{
		CE_ASSERT(path != NULL, "Path must be != NULL");
#if CROWN_PLATFORM_POSIX
		return is_absolute_path(path) && strlen(path) == 1;
#elif CROWN_PLATFORM_WINDOWS
		return is_absolute_path(path) && strlen(path) == 3;
#endif
	}

	void join(const char* p1, const char* p2, DynamicString& path)
	{
		path += p1;
		path += SEPARATOR;
		path += p2;
	}

	const char* normalize_path(const char* path)
	{
#if CROWN_PLATFORM_POSIX
		static char norm[1024];
		char* cur = norm;

		while ((*path) != '\0')
		{
			if ((*path) == '\\')
			{
				(*cur) = SEPARATOR;
			}
			else
			{
				(*cur) = (*path);
			}

			path++;
			cur++;
		}

		return norm;
#elif CROWN_PLATFORM_WINDOWS
		static char norm[1024];

		for (uint32_t i = 0; i < strlen(path)+1; i++)
		{
			if (path[i] == '/')
			{
				norm[i] = SEPARATOR;
			}
			else
			{
				norm[i] = path[i];
			}
		}

		return norm;
#endif
	}

	/// Returns the pathname of the path.
	/// @note
	/// e.g. "/home/project/texture.tga" -> "/home/project"
	/// e.g. "/home/project" -> "/home"
	/// e.g. "/home" -> "/"
	/// e.g. "home" -> ""
	///
	/// The @a path must be valid.
	void pathname(const char* path, char* str, uint32_t len)
	{
		CE_ASSERT(path != NULL, "Path must be != NULL");
		CE_ASSERT(str != NULL, "Str must be != NULL");

		const char* last_separator = strrchr(path, '/');
		const char* end = path + strlen(path) + 1;

		if (last_separator == end)
		{
			strncpy(str, "", len);
		}
		else
		{
			substring(path, last_separator, str, len);
		}
	}

	/// Returns the filename of the path.
	/// @note
	/// e.g. "/home/project/texture.tga" -> "texture.tga"
	/// e.g. "/home/project/texture" -> "texture"
	/// e.g. "/home -> "home"
	/// e.g. "/" -> ""
	///
	/// The @a path must be valid.
	void filename(const char* path, char* str, uint32_t len)
	{
		CE_ASSERT(path != NULL, "Path must be != NULL");
		CE_ASSERT(str != NULL, "Str must be != NULL");

		const char* last_separator = strrchr(path, '/');
		const char* end = str + strlen(path) + 1;

		if (last_separator == end)
		{
			strncpy(str, "", len);
		}
		else
		{
			substring(last_separator + 1, end, str, len);
		}
	}

	/// Returns the basename of the path.
	/// @note
	/// e.g. "/home/project/texture.tga" -> "texture"
	/// e.g. "/home/project" -> "project"
	/// e.g. "/" -> ""
	///
	/// The @a path must be valid.
	void basename(const char* path, char* str, uint32_t len)
	{
		CE_ASSERT(path != NULL, "Path must be != NULL");
		CE_ASSERT(str != NULL, "Str must be != NULL");

		const char* last_separator = strrchr(path, '/');
		const char* last_dot = strrchr(path, '.');
		const char* end = path + strlen(path) + 1;

		if (last_separator == end && last_dot != end)
		{
			substring(path, last_dot, str, len);
		}
		else if (last_separator != end && last_dot == end)
		{
			substring(last_separator + 1, end, str, len);
		}
		else if (last_separator == end && last_dot == end)
		{
			strncpy(str, path, len);
		}
		else
		{
			substring(last_separator + 1, last_dot, str, len);
		}
	}

	/// Returns the extension of the path.
	/// @note
	/// e.g. "/home/project/texture.tga" -> "tga"
	/// e.g. "/home/project.x/texture" -> ""
	///
	/// The @a path must be valid.
	void extension(const char* path, char* str, uint32_t len)
	{
		CE_ASSERT(path != NULL, "Path must be != NULL");
		CE_ASSERT(str != NULL, "Str must be != NULL");

		const char* last_dot = strrchr(path, '.');
		const char* end = path + strlen(path) + 1;

		if (last_dot == end)
		{
			strncpy(str, "", len);
		}
		else
		{
			substring(last_dot + 1, end, str, len);
		}
	}

	/// Returns the filename without the extension.
	/// @note
	/// e.g. "/home/project/texture.tga" -> "/home/project/texture"
	/// e.g. "/home/project/texture" -> "/home/project/texture"
	///
	/// The @a path must be valid.
	void filename_without_extension(const char* path, char* str, uint32_t len)
	{
		CE_ASSERT(path != NULL, "Path must be != NULL");
		CE_ASSERT(str != NULL, "Str must be != NULL");

		const char* last_dot = strrchr(path, '.');

		substring(path, last_dot, str, len);
	}

	/// Fills 'ret' with the same path but without the trailing directory separator.
	/// @note
	/// e.g. "/home/project/texture.tga/" -> "/home/project/texture.tga"
	/// e.g. "/home/project/texture.tga" -> "/home/project/texture.tga"
	///
	/// The @a path must be valid.
	void strip_trailing_separator(const char* path, char* str, uint32_t len)
	{
		CE_ASSERT(path != NULL, "Path must be != NULL");
		CE_ASSERT(str != NULL, "Str must be != NULL");

		size_t path_len = strlen(path);
		const char* end = path + strlen(path) + 1;

		if (path[path_len - 1] == '/')
		{
			substring(path, end - 2, str, len);
		}
		else
		{
			substring(path, end, str, len);
		}
	}
} // namespace path
} // namespace crown
