/*
 * Copyright (c) 2012-2016 Daniele Bartolini and individual contributors.
 * License: https://github.com/taylor001/crown/blob/master/LICENSE
 */

#include "dynamic_string.h"
#include "path.h"
#include <ctype.h> // isalpha
#include <string.h> // strrchr

namespace crown
{
namespace path
{
	bool is_absolute(const char* path)
	{
		CE_ASSERT_NOT_NULL(path);
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
		CE_ASSERT_NOT_NULL(path);
		return !is_absolute(path);
	}

	bool is_root(const char* path)
	{
		CE_ASSERT_NOT_NULL(path);
#if CROWN_PLATFORM_POSIX
		return is_absolute(path) && strlen32(path) == 1;
#elif CROWN_PLATFORM_WINDOWS
		return is_absolute(path) && strlen32(path) == 3;
#endif
	}

	void join(const char* path_a, const char* path_b, DynamicString& path)
	{
		CE_ASSERT_NOT_NULL(path_a);
		CE_ASSERT_NOT_NULL(path_b);
		const u32 la = strlen32(path_a);
		const u32 lb = strlen32(path_b);
		path.reserve(la + lb + 1);
		path += path_a;
		path += PATH_SEPARATOR;
		path += path_b;
	}

	const char* basename(const char* path)
	{
		CE_ASSERT_NOT_NULL(path);
		const char* ls = strrchr(path, '/');
		return ls == NULL ? path : ls + 1;
	}

	const char* extension(const char* path)
	{
		CE_ASSERT_NOT_NULL(path);
		const char* ld = strrchr(path, '.');
		return ld == NULL ? NULL : ld + 1;
	}
} // namespace path
} // namespace crown
