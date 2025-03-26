/*
 * Copyright (c) 2012-2025 Daniele Bartolini et al.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

namespace Crown
{
public class FontResource
{
	public static ImportResult import(ProjectStore project_store, string destination_dir, SList<string> filenames)
	{
		Project project = project_store._project;
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

			GLib.File file_dst       = File.new_for_path(Path.build_filename(destination_dir, file_src.get_basename()));
			string resource_filename = project.resource_filename(file_dst.get_path());
			string resource_path     = ResourceId.normalize(resource_filename);
			string resource_name     = ResourceId.name(resource_path);

			importer_settings_path = project.absolute_path(resource_name) + ".importer_settings";

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
			try {
				importer_settings = SJSON.load_from_path(importer_settings_path);
				dlg.load(importer_settings);
			} catch (JsonSyntaxError e) {
				importer_settings = new Hashtable();
			}
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

			GLib.File file_dst       = File.new_for_path(Path.build_filename(destination_dir, resource_basename));
			string resource_filename = project.resource_filename(file_dst.get_path());
			string resource_path     = ResourceId.normalize(resource_filename);
			string resource_name     = ResourceId.name(resource_path);

			try {
				SJSON.save(importer_settings, project.absolute_path(resource_name) + ".importer_settings");
			} catch (JsonWriteError e) {
				return ImportResult.ERROR;
			}

			// Save .png atlas.
			dlg.save_png(project.absolute_path(resource_name) + ".png");

			Database db = new Database(project);

			// Generate .texture resource.
			var texture_resource = TextureResource.font_atlas(db, Guid.new_guid(), resource_name + ".png");
			texture_resource.save(project, resource_name);
			db.reset();

			// Generate .material resource.
			MaterialResource material_resource = MaterialResource.gui(db, Guid.new_guid(), resource_name);
			if (material_resource.save(project, resource_name) != 0)
				return ImportResult.ERROR;

			// Generate .font resource.
			Guid font_id = Guid.new_guid();
			db.create(font_id, OBJECT_TYPE_FONT);
			db.set_property_double(font_id, "size", size);
			db.set_property_double(font_id, "font_size", font_size);

			for (int ii = 0; ii < dlg._font_range_max.value - dlg._font_range_min.value + 1; ++ii) {
				GlyphData* gd = dlg.glyph_data(ii);

				Guid glyph_id = Guid.new_guid();
				db.create(glyph_id, "font_glyph");
				db.set_property_double(glyph_id, "cp", gd->id);
				db.set_property_double(glyph_id, "x", gd->x);
				db.set_property_double(glyph_id, "y", gd->y);
				db.set_property_double(glyph_id, "width", gd->width);
				db.set_property_double(glyph_id, "height", gd->height);
				db.set_property_double(glyph_id, "x_offset", gd->x_offset);
				db.set_property_double(glyph_id, "y_offset", gd->y_offset);
				db.set_property_double(glyph_id, "x_advance", gd->x_advance);

				db.add_to_set(font_id, "glyphs", glyph_id);
			}

			db.save(project.absolute_path(resource_name) + ".font", font_id);
		}

		dlg.destroy();
		return ImportResult.SUCCESS;
	}
}

} /* namespace Crown */
