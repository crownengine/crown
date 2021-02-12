/*
 * Copyright (c) 2012-2021 Daniele Bartolini et al.
 * License: https://github.com/dbartolini/crown/blob/master/LICENSE
 */

namespace Crown
{
static string basename(string path)
{
	int ls = path.last_index_of_char('/');
	return ls == -1 ? path : path.substring(ls+1);
}

static string? extension(string path)
{
	string bn = basename(path);
	int ld = bn.last_index_of_char('.');
	return (ld == -1 || bn.substring(ld) == bn) ? null : bn.substring(ld+1);
}

static string? resource_type(string path)
{
	return extension(path);
}

static string? resource_name(string? type, string path)
{
	return type == null ? null : path.substring(0, path.last_index_of_char('.'));
}

}
