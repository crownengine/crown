/*
 * Copyright (c) 2012-2026 Daniele Bartolini et al.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

namespace Crown
{
namespace ResourceId
{
	/// Returns the extension of @a path or null if the path has no extension.
	public string? extension(string path)
	{
		string bn = GLib.Path.get_basename(path);
		int ld = bn.last_index_of_char('.');
		return (ld == -1 || bn.substring(ld) == bn) ? null : bn.substring(ld + 1);
	}

	/// Returns the type of the resource @a path or null if the path has not type.
	public string? type(string path)
	{
		return ResourceId.extension(path);
	}

	/// Returns the name of the resource @a path or null if the path is not a resource path.
	public string? name(string path)
	{
		string? type = ResourceId.type(path);
		return type == null ? null : path.substring(0, path.last_index_of_char('.'));
	}

	/// Returns the parent folder of the resource located at @a path.
	public string parent_folder(string path)
	{
		int ls = path.last_index_of_char('/');

		if (ls == -1)
			return "";

		return path.substring(0, ls);
	}

	public string path(string type, string name)
	{
		return type == "" ? name : name + "." + type;
	}

	/// Converts a regular path into a resource path.
	public string normalize(string path)
	{
		return path.replace("\\", "/");
	}

} /* namespace ResourceId */

} /* namespace Crown */
