/*
 * Copyright (c) 2012-2026 Daniele Bartolini et al.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

namespace Crown
{
public static string path_extension(string path)
{
	string bn = GLib.Path.get_basename(path);
	int ld = bn.last_index_of(".");
	return (ld == -1 || ld == 0) ? "" : bn.substring(ld + 1);
}

} /* namespace Crown */
