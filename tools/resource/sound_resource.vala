/*
 * Copyright (c) 2012-2024 Daniele Bartolini et al.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

using Gee;

namespace Crown
{
public class SoundResource
{
	public static ImportResult import(Project project, string destination_dir, SList<string> filenames)
	{
		foreach (unowned string filename_i in filenames) {
			GLib.File file_src = File.new_for_path(filename_i);

			GLib.File file_dst       = File.new_for_path(Path.build_filename(destination_dir, file_src.get_basename()));
			string resource_filename = project.resource_filename(file_dst.get_path());
			string resource_path     = ResourceId.normalize(resource_filename);
			string resource_name     = ResourceId.name(resource_path);

			try {
				file_src.copy(file_dst, FileCopyFlags.OVERWRITE);
			} catch (Error e) {
				loge(e.message);
				return ImportResult.ERROR;
			}

			Hashtable sound = new Hashtable();
			sound["source"] = resource_name;

			SJSON.save(sound, project.absolute_path(resource_name) + ".sound");
		}

		return ImportResult.SUCCESS;
	}
}

} /* namespace Crown */
