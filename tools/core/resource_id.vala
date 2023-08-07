/*
 * Copyright (c) 2012-2023 Daniele Bartolini et al.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

namespace Crown
{
namespace ResourceId
{
	/// Returns the name of the resource at @a path.
	public string resource_name(string path)
	{
		return path.replace("\\", "/");
	}

	public string resource_path(string resource_filename)
	{
		return resource_filename.substring(0
			, resource_filename.last_index_of_char('.')
			);
	}

} /* namespace ResourceId */

} /* namespace Crown */
