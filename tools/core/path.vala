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

public static bool is_directory_empty(string path)
{
	GLib.File file = GLib.File.new_for_path(path);
	try {
		FileEnumerator enumerator = file.enumerate_children("standard::*"
			, FileQueryInfoFlags.NOFOLLOW_SYMLINKS
			);
		return enumerator.next_file() == null;
	} catch (GLib.Error e) {
		loge(e.message);
	}

	return false;
}

public static int copy_tree(GLib.File dst, GLib.File src, GLib.FileCopyFlags flags = GLib.FileCopyFlags.NONE)
{
	try {
		GLib.FileType src_type = src.query_file_type(GLib.FileQueryInfoFlags.NONE);
		if (src_type == GLib.FileType.DIRECTORY) {
			if (dst.query_exists() == false)
				dst.make_directory();

			string dst_path = dst.get_path();
			string src_path = src.get_path();
			GLib.FileEnumerator fe = src.enumerate_children(GLib.FileAttribute.STANDARD_NAME, GLib.FileQueryInfoFlags.NONE);
			for (GLib.FileInfo? info = fe.next_file(); info != null; info = fe.next_file()) {
				if (copy_tree(GLib.File.new_for_path(GLib.Path.build_filename(dst_path, info.get_name()))
					, GLib.File.new_for_path(GLib.Path.build_filename(src_path, info.get_name()))
					, flags
					) != 0) {
					fe.close(null);
					return -1;
				}
			}
		} else if (src_type == GLib.FileType.REGULAR) {
			src.copy(dst, flags);
		}
	} catch (GLib.Error e) {
		loge(e.message);
		return -1;
	}

	return 0;
}

} /* namespace Crown */
