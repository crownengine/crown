/*
 * Copyright (c) 2012-2023 Daniele Bartolini et al.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

namespace Crown
{
public class FontResource
{
	public static ImportResult import(Project project, string destination_dir, SList<string> filenames)
	{
		Hashtable importer_settings = null;
		string importer_settings_path = null;
		string font_path;
		string font_type;
		string font_name;
		{
			GLib.File file_src = File.new_for_path(filenames.nth_data(0));
			font_path = file_src.get_path();
			font_type = font_path.substring(font_path.last_index_of_char('.') + 1
				, font_path.length - font_path.last_index_of_char('.') - 1
				);

			GLib.File file_dst = File.new_for_path(Path.build_filename(destination_dir, file_src.get_basename()));
			string resource_filename = project._source_dir.get_relative_path(file_dst);
			string resource_path     = ResourceId.normalize(resource_filename);
			string resource_name     = ResourceId.name(resource_path);

			importer_settings_path = Path.build_filename(project.source_dir(), resource_name) + ".importer_settings";

			font_path = file_src.get_path();

			int last_slash = resource_name.last_index_of_char('/');
			if (last_slash == -1)
				font_name = resource_name;
			else
				font_name = resource_name.substring(last_slash + 1, resource_name.length - last_slash - 1);
		}

		FontImportDialog dlg = new FontImportDialog(font_path, font_name);
		dlg._font_name.sensitive = filenames.length() == 1;
		dlg.show_all();

		if (File.new_for_path(importer_settings_path).query_exists()) {
			importer_settings = SJSON.load_from_path(importer_settings_path);
			dlg.load(importer_settings);
		} else {
			importer_settings = new Hashtable();
		}

		dlg.generate_atlas();

		if (dlg.run() != Gtk.ResponseType.OK) {
			dlg.destroy();
			return ImportResult.CANCEL;
		}

		dlg.save(importer_settings);

		int size      = (int)dlg.atlas_size();
		int font_size = (int)dlg._font_size.value;
		font_name     = dlg._font_name.value;

		dlg.hide();

		foreach (unowned string filename_i in filenames) {
			GLib.File file_src = File.new_for_path(filename_i);
			string resource_basename;
			if (filenames.length() == 1)
				resource_basename = font_name + "." + font_type;
			else
				resource_basename = file_src.get_basename();

			GLib.File file_dst = File.new_for_path(Path.build_filename(destination_dir, resource_basename));
			string resource_filename = project._source_dir.get_relative_path(file_dst);
			string resource_path     = ResourceId.normalize(resource_filename);
			string resource_name     = ResourceId.name(resource_path);

			SJSON.save(importer_settings, Path.build_filename(project.source_dir(), resource_name) + ".importer_settings");

			// Save .png atlas.
			dlg.save_png(Path.build_filename(project.source_dir(), resource_name + ".png"));

			Database db = new Database(project);

			// Generate .texture resource.
			Guid texture_id = Guid.new_guid();
			db.create(texture_id, "texture");
			db.set_property_string(texture_id, "source", resource_name + ".png");
			db.set_property_bool  (texture_id, "generate_mips", false);
			db.set_property_bool  (texture_id, "normal_map", false);

			db.save(Path.build_filename(project.source_dir(), resource_name) + ".texture", texture_id);
			db.reset();

			// Generate .material resource.
			Hashtable textures = new Hashtable();
			textures["u_albedo"] = resource_name;

			Hashtable material = new Hashtable();
			material["shader"]   = "gui+DIFFUSE_MAP";
			material["textures"] = textures;
			SJSON.save(material, Path.build_filename(project.source_dir(), resource_name) + ".material");

			// Generate .font resource.
			var glyphs = new Gee.ArrayList<Value?>();
			for (int ii = 0; ii < dlg._font_range_max.value - dlg._font_range_min.value + 1; ++ii) {
				GlyphData* gd = dlg.glyph_data(ii);

				Hashtable glyph = new Hashtable();
				glyph["id"]        = gd->id;
				glyph["x"]         = gd->x;
				glyph["y"]         = gd->y;
				glyph["width"]     = gd->width;
				glyph["height"]    = gd->height;
				glyph["x_offset"]  = gd->x_offset;
				glyph["y_offset"]  = gd->y_offset;
				glyph["x_advance"] = gd->x_advance;
				glyphs.add(glyph);
			}

			Hashtable font = new Hashtable();
			font["size"]      = size;
			font["font_size"] = font_size;
			font["glyphs"]    = glyphs;
			SJSON.save(font, Path.build_filename(project.source_dir(), resource_name) + ".font");
		}

		dlg.destroy();
		return ImportResult.SUCCESS;
	}
}

} /* namespace Crown */
