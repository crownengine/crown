/*
 * Copyright (c) 2012-2022 Daniele Bartolini et al.
 * License: https://github.com/crownengine/crown/blob/master/LICENSE
 */

using Gee;

namespace Crown
{
public class TextureResource
{
	public static int import(Project project, string destination_dir, SList<string> filenames)
	{
		foreach (unowned string filename_i in filenames) {
			GLib.File file_src = File.new_for_path(filename_i);
			GLib.File file_dst = File.new_for_path(Path.build_filename(destination_dir, file_src.get_basename()));

			string resource_filename = project._source_dir.get_relative_path(file_dst);
			string resource_path     = resource_filename.substring(0, resource_filename.last_index_of_char('.'));

			try {
				file_src.copy(file_dst, FileCopyFlags.OVERWRITE);
			}
			catch (Error e) {
				loge(e.message);
			}

			Database db = new Database(project);

			Guid texture_id = Guid.new_guid();
			db.create(texture_id, "texture");
			db.set_property_string(texture_id, "source", project.resource_path_to_resource_name(resource_filename));
			db.set_property_bool  (texture_id, "generate_mips", true);
			db.set_property_bool  (texture_id, "normal_map", false);

			db.save(Path.build_filename(project.source_dir(), resource_path + ".texture"), texture_id);
		}

		return 0;
	}
}

} /* namespace Crown */
