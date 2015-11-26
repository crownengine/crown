/*
 * Copyright (c) 2012-2015 Daniele Bartolini and individual contributors.
 * License: https://github.com/taylor001/crown/blob/master/LICENSE
 */

#include "path.h"
#include <ctype.h> // isalpha
#include <string.h> // strlen, strrchr

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

	const char* basename(const char* path)
	{
		const char* ls = strrchr(path, '/');
		return ls == NULL ? path : ls + 1;
	}

	const char* extension(const char* path)
	{
		const char* ld = strrchr(path, '.');
		return ld == NULL ? NULL : ld + 1;
	}
} // namespace path
} // namespace crown
