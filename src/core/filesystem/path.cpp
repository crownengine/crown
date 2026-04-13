/*
 * Copyright (c) 2012-2026 Daniele Bartolini et al.
 * SPDX-License-Identifier: MIT
 */

#include "core/containers/array.inl"
#include "core/date.h"
#include "core/environment.h"
#include "core/filesystem/path.h"
#include "core/math/random.inl"
#include "core/memory/temp_allocator.inl"
#include "core/platform.h"
#include "core/strings/dynamic_string.inl"
#include "core/strings/string_view.inl"
#include "device/types.h"
#include <ctype.h>  // isalpha, isalnum
#include <string.h> // strncmp, strrchr

namespace crown
{
extern PlatformData g_platform_data;

#if CROWN_PLATFORM_WINDOWS
const char PATH_SEPARATOR = '\\';
#else
const char PATH_SEPARATOR = '/';
#endif

namespace path
{
	bool is_absolute(const char *path)
	{
		CE_ENSURE(NULL != path);
#if CROWN_PLATFORM_WINDOWS
		return (strlen32(path) > 2 && isalpha(path[0]) && path[1] == ':' && path[2] == PATH_SEPARATOR)
			|| (strlen32(path) > 1 && path[0] == '/' && isalpha(path[1]))
			;
#else
		return strlen32(path) > 0
			&& path[0] == PATH_SEPARATOR
			;
#endif
	}

	bool is_relative(const char *path)
	{
		CE_ENSURE(NULL != path);
		return !is_absolute(path);
	}

	bool is_root(const char *path)
	{
		CE_ENSURE(NULL != path);
#if CROWN_PLATFORM_WINDOWS
		return is_absolute(path)
			&& ((strlen(path) == 2 && path[0] == '/') || strlen32(path) == 3 && isalpha(path[0]));
#else
		return is_absolute(path) && strlen32(path) == 1;
#endif
	}

	void join(DynamicString &path, const StringView &path_a, const StringView &path_b)
	{
		const u32 la = path_a.length();
		const u32 lb = path_b.length();
		path.reserve(la + lb + 1);
		path  = path_a;
		if (la != 0 && lb != 0)
			path += PATH_SEPARATOR;
		path += path_b;
	}

	void join(DynamicString &path, const char *path_a, const char *path_b)
	{
		join(path, StringView(path_a), StringView(path_b));
	}

	const char *basename(const char *path)
	{
		CE_ENSURE(NULL != path);
		const char *ls = strrchr(path, '/');
		return ls == NULL ? path : ls + 1;
	}

	const char *extension(const char *path)
	{
		CE_ENSURE(NULL != path);
		const char *bn = basename(path);
		const char *ld = strrchr(bn, '.');
		return (ld == NULL || ld == bn) ? NULL : ld + 1;
	}

	bool has_trailing_separator(const char *path)
	{
		CE_ENSURE(NULL != path);
		return path[strlen32(path) - 1] == PATH_SEPARATOR;
	}

	inline bool any_separator(char c)
	{
		return c == '/' || c == '\\';
	}

	void reduce(DynamicString &clean, const char *path)
	{
		if (path == NULL)
			return;

		char cc = any_separator(*path) ? PATH_SEPARATOR : *path;
		clean += cc;
		++path;

		for (; *path; ++path) {
			if (cc == PATH_SEPARATOR && any_separator(*path))
				continue;

			cc = any_separator(*path) ? PATH_SEPARATOR : *path;
			clean += cc;
		}

		if (has_trailing_separator(clean.c_str()))
			array::pop_back(clean._data);
	}

	bool expand(DynamicString &expanded, const char *path_template)
	{
		const char *ch = path_template;
		CE_ENSURE(ch != NULL);

		expanded = "";
		char buf[16];

		while (*ch) {
			if (*ch != '$') {
				expanded += *ch;
				++ch;
				continue;
			}

			++ch;
			if (strncmp(ch, "USER_DATA", 9) == 0 && !isalnum((unsigned char)ch[9])) {
				ch += 9;
				if (CROWN_PLATFORM_ANDROID) {
					const char *path = (const char *)g_platform_data._android_internal_data_path;
					if (path == NULL || path[0] == '\0')
						return false;
					expanded += path;
				} else {
					TempAllocator256 ta;
					DynamicString path(ta);
					environment::user_data_dir(path);
					if (path.empty())
						return false;

					expanded += path;
				}
			} else if (CROWN_PLATFORM_ANDROID
				&& strncmp(ch, "OBB_PATH", 8) == 0
				&& !isalnum((unsigned char)ch[8])
				) {
				ch += 8;
				const char *path = (const char *)g_platform_data._android_obb_path;
				if (path == NULL || path[0] == '\0')
					return false;
				expanded += path;
			} else if (strncmp(ch, "UTC_DATE", 8) == 0 && !isalnum((unsigned char)ch[8])) {
				ch += 8;
				date::Date d;
				date::utc_date(d);
				date::to_string(buf, sizeof(buf), d);
				expanded += buf;
			} else if (strncmp(ch, "UTC_TIME", 8) == 0 && !isalnum((unsigned char)ch[8])) {
				ch += 8;
				date::Time t;
				date::utc_time(t);
				date::to_string(buf, sizeof(buf), t);
				expanded += buf;
			} else if (strncmp(ch, "RANDOM", 6) == 0 && !isalnum((unsigned char)ch[6])) {
				ch += 6;
				Random random;
				const char alphabet[] = "0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";
				for (u32 ii = 0; ii < 8; ++ii)
					buf[ii] = alphabet[random.integer(s32(sizeof(alphabet) - 1))];
				buf[8] = '\0';
				expanded += buf;
			} else if (strncmp(ch, "DATE", 4) == 0 && !isalnum((unsigned char)ch[4])) {
				ch += 4;
				date::Date d;
				date::date(d);
				date::to_string(buf, sizeof(buf), d);
				expanded += buf;
			} else if (strncmp(ch, "TIME", 4) == 0 && !isalnum((unsigned char)ch[4])) {
				ch += 4;
				date::Time t;
				date::time(t);
				date::to_string(buf, sizeof(buf), t);
				expanded += buf;
			} else if (strncmp(ch, "TMP", 3) == 0 && !isalnum((unsigned char)ch[3])) {
				ch += 3;
				TempAllocator256 ta;
				DynamicString path(ta);
				environment::tmp_dir(path);
				if (path.empty())
					return false;

				expanded += path;
			} else {
				return false;
			}
		}

		return true;
	}

	StringView parent_dir(const char *path)
	{
		const char *ls = strrchr(path, PATH_SEPARATOR);

		if (!ls)
			return { NULL, 0 };

#if CROWN_PLATFORM_WINDOWS
		if (ls == path + 2 && path[1] == ':')
			return { path, 3 };
#else
		if (ls == path)
			return { path, 1 };
#endif

		return { path, u32(ls - path) };
	}

} // namespace path

} // namespace crown
