/*
 * Copyright (c) 2012-2021 Daniele Bartolini et al.
 * License: https://github.com/dbartolini/crown/blob/master/LICENSE
 */

#include "core/containers/array.inl"
#include "core/filesystem/path.h"
#include "core/platform.h"
#include "core/strings/dynamic_string.inl"
#include <ctype.h>  // isalpha
#include <string.h> // strrchr

namespace crown
{
#if CROWN_PLATFORM_POSIX
const char PATH_SEPARATOR = '/';
#elif CROWN_PLATFORM_WINDOWS
const char PATH_SEPARATOR = '\\';
#endif // CROWN_PLATFORM_POSIX

namespace path
{
	bool is_absolute(const char* path)
	{
		CE_ENSURE(NULL != path);
#if CROWN_PLATFORM_POSIX
		return strlen32(path) > 0
			&& path[0] == PATH_SEPARATOR
			;
#elif CROWN_PLATFORM_WINDOWS
		return strlen32(path) > 2
			&& isalpha(path[0])
			&& path[1] == ':'
			&& path[2] == PATH_SEPARATOR
			;
#endif
	}

	bool is_relative(const char* path)
	{
		CE_ENSURE(NULL != path);
		return !is_absolute(path);
	}

	bool is_root(const char* path)
	{
		CE_ENSURE(NULL != path);
#if CROWN_PLATFORM_POSIX
		return is_absolute(path) && strlen32(path) == 1;
#elif CROWN_PLATFORM_WINDOWS
		return is_absolute(path) && strlen32(path) == 3;
#endif
	}

	void join(DynamicString& path, const char* path_a, const char* path_b)
	{
		CE_ENSURE(NULL != path_a);
		CE_ENSURE(NULL != path_b);
		const u32 la = strlen32(path_a);
		const u32 lb = strlen32(path_b);
		path.reserve(la + lb + 1);
		path  = path_a;
		if (la != 0 && lb != 0)
			path += PATH_SEPARATOR;
		path += path_b;
	}

	const char* basename(const char* path)
	{
		CE_ENSURE(NULL != path);
		const char* ls = strrchr(path, '/');
		return ls == NULL ? path : ls + 1;
	}

	const char* extension(const char* path)
	{
		CE_ENSURE(NULL != path);
		const char* bn = basename(path);
		const char* ld = strrchr(bn, '.');
		return (ld == NULL || ld == bn) ? NULL : ld + 1;
	}

	bool has_trailing_separator(const char* path)
	{
		CE_ENSURE(NULL != path);
		return path[strlen32(path) - 1] == PATH_SEPARATOR;
	}

	inline bool any_separator(char c)
	{
		return c == '/' || c == '\\';
	}

	void reduce(DynamicString& clean, const char* path)
	{
		if (path == NULL)
			return;

		char cc = any_separator(*path) ? PATH_SEPARATOR : *path;
		clean += cc;
		++path;

		for (; *path; ++path)
		{
			if (cc == PATH_SEPARATOR && any_separator(*path))
				continue;

			cc = any_separator(*path) ? PATH_SEPARATOR : *path;
			clean += cc;
		}

		if (has_trailing_separator(clean.c_str()))
			array::pop_back(clean._data);
	}

} // namespace path

} // namespace crown
