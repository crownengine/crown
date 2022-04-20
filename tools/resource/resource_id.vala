/*
 * Copyright (c) 2012-2022 Daniele Bartolini et al.
 * License: https://github.com/dbartolini/crown/blob/master/LICENSE
 */

namespace Crown
{
static string basename(string path)
{
	int ls = path.last_index_of_char('/');
	return ls == -1 ? path : path.substring(ls+1);
}

/// Returns the extension of @a path or null if the path has no extension.
static string? extension(string path)
{
	string bn = basename(path);
	int ld = bn.last_index_of_char('.');
	return (ld == -1 || bn.substring(ld) == bn) ? null : bn.substring(ld+1);
}

/// Returns the type of the resource @a path or null if the path has not type.
static string? resource_type(string path)
{
	return extension(path);
}

/// Returns the name of the resource @a path or null if the path is not a resource path.
static string? resource_name(string? type, string path)
{
	return type == null ? null : path.substring(0, path.last_index_of_char('.'));
}

static string resource_path(string type, string name)
{
	return type == "" ? name : name + "." + type;
}

} /* namespace Crown */
