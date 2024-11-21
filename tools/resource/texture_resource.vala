/*
 * Copyright (c) 2012-2024 Daniele Bartolini et al.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

using Gee;

namespace Crown
{
public enum TextureFormat
{
	BC1,
	BC2,
	BC3,
	BC4,
	BC5,
	PTC14,
	RGB8,
	RGBA8,

	COUNT;

	public string to_key()
	{
		switch (this) {
		case BC1:
			return "BC1";
		case BC2:
			return "BC2";
		case BC3:
			return "BC3";
		case BC4:
			return "BC4";
		case BC5:
			return "BC5";
		case PTC14:
			return "PTC14";
		case RGB8:
			return "RGB8";
		case RGBA8:
			return "RGBA8";
		default:
			return "BC1";
		}
	}
}

public class TextureResource
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

			Database db = new Database(project);

			Guid texture_id = Guid.new_guid();
			db.create(texture_id, "texture");
			db.set_property_string(texture_id, "source", resource_path);
			db.set_property_bool  (texture_id, "generate_mips", true);
			db.set_property_bool  (texture_id, "normal_map", false);

			db.save(project.absolute_path(resource_name) + ".texture", texture_id);
		}

		return ImportResult.SUCCESS;
	}
}

} /* namespace Crown */
