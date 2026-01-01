/*
 * Copyright (c) 2012-2026 Daniele Bartolini et al.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

namespace Crown
{
[SimpleType]
[CCode (cname = "GlyphData", has_type_id = false)]
public struct GlyphData
{
	int id;
	int x;         ///< X-position inside the atlas.
	int y;         ///< Y-position inside the atlas.
	int width;     ///< In pixels.
	int height;    ///< In pixels.
	int x_offset;  ///< In pixels.
	int y_offset;  ///< In pixels.
	int x_advance; ///< In pixels.
}

[SimpleType]
[CCode (cname = "FontAtlas", has_type_id = false)]
public struct FontAtlas
{
	uchar* image_data;
	int size;
	GlyphData* glyphs;
	int num_glyphs;
}

[CCode (cname = "crown_font_atlas_free")]
extern void font_atlas_free(FontAtlas* atlas);

[CCode (cname = "crown_font_atlas_generate")]
extern void* font_atlas_generate(string font_path, int font_size, int range_min, int range_max);

public enum FontChars
{
	ASCII_PRINTABLE,
	ASCII_NUMBERS,
	ASCII_LETTERS,
	CUSTOM_RANGE
}

// Copies @a src alpha to @a dst BRGA channels.
public void copy_alpha_to_argb32(Cairo.ImageSurface dst, Cairo.ImageSurface src)
{
	unowned uchar[] dst_data = dst.get_data();
	unowned uchar[] src_data = src.get_data();
	int src_stride = src.get_stride();
	int dst_stride = dst.get_stride();
	int width  = src.get_width();
	int height = src.get_height();

	for (int yy = 0; yy < height; yy++) {
		for (int xx = 0; xx < width; xx++) {
			uint8 alpha = src_data[yy * src_stride + xx];
			dst_data[yy * dst_stride + 4 * xx + 0] = alpha; // B
			dst_data[yy * dst_stride + 4 * xx + 1] = alpha; // G
			dst_data[yy * dst_stride + 4 * xx + 2] = alpha; // R
			dst_data[yy * dst_stride + 4 * xx + 3] = alpha; // A
		}
	}
}

public class FontImportDialog : Gtk.Window
{
	public Project _project;
	public string _destination_dir;
	public GLib.SList<string> _filenames;
	public unowned Import _import_result;

	public string _font_type;

	public FontAtlas* _font_atlas;
	public Cairo.ImageSurface _atlas;
	public PixbufView _drawing_area;
	public Gtk.ScrolledWindow _scrolled_window;

	public Gtk.Label _atlas_size;
	public Gtk.Label _font_path;
	public InputResourceBasename _font_name;
	public InputDouble _font_size;
	public Gtk.ComboBoxText _font_chars;
	public InputDouble _font_range_min;
	public InputDouble _font_range_max;

	public Gtk.Box _box;
	public Gtk.Button _import;
	public Gtk.Button _cancel;
	public Gtk.HeaderBar _header_bar;

	public void set_font_range(int min, int max)
	{
		if (min > max || max < min)
			max = min;

		_font_range_min.value = min;
		_font_range_max.value = max;
	}

	public int atlas_size()
	{
		return _font_atlas.size;
	}

	public void generate_atlas()
	{
		font_atlas_free(_font_atlas);
		_font_atlas = (FontAtlas*)font_atlas_generate(_font_path.get_text()
			, (int)_font_size.value
			, (int)_font_range_min.value
			, (int)_font_range_max.value
			);

		_atlas = new Cairo.ImageSurface.for_data((uchar[])_font_atlas.image_data
			, Cairo.Format.A8
			, _font_atlas.size
			, _font_atlas.size
			, Cairo.Format.A8.stride_for_width(_font_atlas.size)
			);

		_atlas_size.set_text(_atlas.get_width().to_string() + " × " + _atlas.get_height().to_string());
		_drawing_area.set_pixbuf(Gdk.pixbuf_get_from_surface(_atlas, 0, 0, _atlas.get_width(), _atlas.get_height()));
		_drawing_area._zoom = 1.0;
	}

	public GlyphData* glyph_data(int index)
	{
		return &_font_atlas.glyphs[index];
	}

	public void save_png(string path)
	{
		var argb32 = new Cairo.ImageSurface(Cairo.Format.ARGB32
			, _atlas.get_width()
			, _atlas.get_height()
			);
		copy_alpha_to_argb32(argb32, _atlas);
		argb32.write_to_png(path);
	}

	public FontImportDialog(Database database, string destination_dir, GLib.SList<string> filenames, Import import_result)
	{
		this.set_icon_name(CROWN_EDITOR_ICON_NAME);

		_project = database._project;
		_destination_dir = destination_dir;
		_filenames = new GLib.SList<string>();
		foreach (var f in filenames)
			_filenames.append(f);
		_import_result = import_result;

		string settings_path;
		string font_name;
		string font_path;
		{
			GLib.File file_src = File.new_for_path(filenames.nth_data(0));
			font_path = file_src.get_path();
			_font_type = font_path.substring(font_path.last_index_of_char('.') + 1
				, font_path.length - font_path.last_index_of_char('.') - 1
				);

			GLib.File file_dst       = File.new_for_path(Path.build_filename(destination_dir, file_src.get_basename()));
			string resource_filename = _project.resource_filename(file_dst.get_path());
			string resource_path     = ResourceId.normalize(resource_filename);
			string resource_name     = ResourceId.name(resource_path);

			settings_path = _project.absolute_path(resource_name) + ".importer_settings";

			font_path = file_src.get_path();

			int last_slash = resource_name.last_index_of_char('/');
			if (last_slash == -1)
				font_name = resource_name;
			else
				font_name = resource_name.substring(last_slash + 1, resource_name.length - last_slash - 1);
		}

		_drawing_area = new PixbufView();
		_drawing_area._filter = Cairo.Filter.BILINEAR;
		_drawing_area._extend = Cairo.Extend.NONE;

		_scrolled_window = new Gtk.ScrolledWindow(null, null);
		_scrolled_window.min_content_width = 640;
		_scrolled_window.min_content_height = 640;
		_scrolled_window.add(_drawing_area);

		_atlas_size = new Gtk.Label("? × ?");
		_atlas_size.halign = Gtk.Align.START;
		_font_path = new Gtk.Label(font_path);
		_font_name = new InputResourceBasename(font_name);
		_font_name.sensitive = filenames.length() == 1;
		_font_size = new InputDouble(24.0, 1.0, 999.0);
		_font_range_min = new InputDouble(32.0, 0.0, int32.MAX);
		_font_range_min.sensitive = false;
		_font_range_max = new InputDouble(126.0, 0.0, int32.MAX);
		_font_range_max.sensitive = false;
		_font_chars = new Gtk.ComboBoxText();
		_font_chars.append_text("ASCII Printable"); // FontChars.ASCII_PRINTABLE
		_font_chars.append_text("ASCII Numbers");   // FontChars.ASCII_NUMBERS
		_font_chars.append_text("ASCII Letters");   // FontChars.ASCII_LETTERS
		_font_chars.append_text("Custom Range");    // FontChars.CUSTOM_RANGE
		_font_chars.active = FontChars.ASCII_PRINTABLE;
		_font_chars.changed.connect(() => {
				// code-format off
				switch (_font_chars.active) {
				case FontChars.ASCII_PRINTABLE:
					set_font_range(32, 126);
					_font_range_min.sensitive = false;
					_font_range_max.sensitive = false;
					break;
				case FontChars.ASCII_NUMBERS:
					set_font_range(48, 57);
					_font_range_min.sensitive = false;
					_font_range_max.sensitive = false;
					break;
				case FontChars.ASCII_LETTERS:
					set_font_range(97, 122);
					_font_range_min.sensitive = false;
					_font_range_max.sensitive = false;
					break;
				case FontChars.CUSTOM_RANGE:
					_font_range_min.sensitive = true;
					_font_range_max.sensitive = true;
					break;
				default:
					break;
				}
				// code-format on

				generate_atlas();
				_drawing_area.queue_draw();
			});

		_font_size.value_changed.connect(() => {
				generate_atlas();
				_drawing_area.queue_draw();
			});

		_font_range_min.value_changed.connect(() => {
				set_font_range((int)_font_range_min.value, (int)_font_range_max.value);
				generate_atlas();
				_drawing_area.queue_draw();
			});
		_font_range_max.value_changed.connect(() => {
				set_font_range((int)_font_range_min.value, (int)_font_range_max.value);
				generate_atlas();
				_drawing_area.queue_draw();
			});

		PropertyGridSet sprite_set = new PropertyGridSet();

		PropertyGrid cv;
		cv = new PropertyGrid();
		cv.add_row("Source path", _font_path);
		sprite_set.add_property_grid(cv, "File");

		cv = new PropertyGrid();
		cv.add_row("Name", _font_name);
		cv.add_row("Atlas size", _atlas_size);
		cv.add_row("Size", _font_size);
		cv.add_row("Charset", _font_chars);
		cv.add_row("Range min", _font_range_min);
		cv.add_row("Range max", _font_range_max);
		sprite_set.add_property_grid(cv, "Font");

		Gtk.Box box = new Gtk.Box(Gtk.Orientation.HORIZONTAL, 0);
		box.pack_start(_scrolled_window, true, true);

		Gtk.Paned pane;
		pane = new Gtk.Paned(Gtk.Orientation.HORIZONTAL);
		pane.pack1(box, false, false);
		pane.pack2(sprite_set, true, false);

		this.destroy.connect(on_destroy);
		this.map_event.connect(on_map_event);

		_box = new Gtk.Box(Gtk.Orientation.VERTICAL, 0);
		_box.pack_start(pane, false, false);

		_cancel = new Gtk.Button.with_label("Cancel");
		_cancel.clicked.connect(() => {
				close();
			});
		_import = new Gtk.Button.with_label("Import");
		_import.get_style_context().add_class("suggested-action");
		_import.clicked.connect(on_import);

		_header_bar = new Gtk.HeaderBar();
		_header_bar.title = "Import Font...";
		_header_bar.show_close_button = true;
		_header_bar.pack_start(_cancel);
		_header_bar.pack_end(_import);

		this.set_titlebar(_header_bar);
		this.add(_box);

		if (File.new_for_path(settings_path).query_exists()) {
			try {
				decode(SJSON.load_from_path(settings_path));
			} catch (JsonSyntaxError e) {
				// No-op.
			}
		}

		generate_atlas();
	}

	public bool on_map_event(Gdk.EventAny ev)
	{
		_font_name.grab_focus();
		return Gdk.EVENT_PROPAGATE;
	}

	public void on_destroy()
	{
		font_atlas_free(_font_atlas);
	}

	public void decode(Hashtable obj)
	{
		_font_chars.active = FontChars.CUSTOM_RANGE;
		_font_size.value = (double)obj["size"];
		set_font_range((int)(double)obj["range_min"], (int)(double)obj["range_max"]);
	}

	public Hashtable encode()
	{
		Hashtable obj = new Hashtable();

		obj["size"]      = _font_size.value;
		obj["range_min"] = _font_range_min.value;
		obj["range_max"] = _font_range_max.value;

		return obj;
	}

	public void on_import()
	{
		_import_result(FontResource.do_import(this, _project, _destination_dir, _filenames));
		destroy();
	}
}

public class FontResource
{
	public static ImportResult do_import(FontImportDialog dlg, Project project, string destination_dir, GLib.SList<string> filenames)
	{
		int size      = (int)dlg.atlas_size();
		int font_size = (int)dlg._font_size.value;

		foreach (unowned string filename_i in filenames) {
			GLib.File file_src = File.new_for_path(filename_i);
			string resource_basename;
			if (filenames.length() == 1)
				resource_basename = dlg._font_name.value + "." + dlg._font_type;
			else
				resource_basename = file_src.get_basename();

			GLib.File file_dst       = File.new_for_path(Path.build_filename(destination_dir, resource_basename));
			string resource_filename = project.resource_filename(file_dst.get_path());
			string resource_path     = ResourceId.normalize(resource_filename);
			string resource_name     = ResourceId.name(resource_path);

			try {
				SJSON.save(dlg.encode(), project.absolute_path(resource_name) + ".importer_settings");
			} catch (JsonWriteError e) {
				return ImportResult.ERROR;
			}

			// Save .png atlas.
			dlg.save_png(project.absolute_path(resource_name) + ".png");

			Database db = new Database(project);

			// Generate .texture resource.
			var texture_resource = TextureResource.font_atlas(db, Guid.new_guid(), resource_name + ".png");
			if (texture_resource.save(project, resource_name) != 0)
				return ImportResult.ERROR;

			db.reset();

			// Generate .material resource.
			MaterialResource material_resource = MaterialResource.gui(db, Guid.new_guid(), resource_name);
			if (material_resource.save(project, resource_name) != 0)
				return ImportResult.ERROR;

			// Generate .font resource.
			Guid font_id = Guid.new_guid();
			db.create(font_id, OBJECT_TYPE_FONT);
			db.set_double(font_id, "size", size);
			db.set_double(font_id, "font_size", font_size);

			for (int ii = 0; ii < dlg._font_range_max.value - dlg._font_range_min.value + 1; ++ii) {
				GlyphData* gd = dlg.glyph_data(ii);

				Guid glyph_id = Guid.new_guid();
				db.create(glyph_id, "font_glyph");
				db.set_double(glyph_id, "cp", gd->id);
				db.set_double(glyph_id, "x", gd->x);
				db.set_double(glyph_id, "y", gd->y);
				db.set_double(glyph_id, "width", gd->width);
				db.set_double(glyph_id, "height", gd->height);
				db.set_double(glyph_id, "x_offset", gd->x_offset);
				db.set_double(glyph_id, "y_offset", gd->y_offset);
				db.set_double(glyph_id, "x_advance", gd->x_advance);

				db.add_to_set(font_id, "glyphs", glyph_id);
			}

			if (db.save(project.absolute_path(resource_name) + ".font", font_id) != 0)
				return ImportResult.ERROR;
		}

		dlg.destroy();
		return ImportResult.SUCCESS;
	}

	public static void import(Import import_result, Database database, string destination_dir, SList<string> filenames, Gtk.Window? parent_window)
	{
		FontImportDialog dlg = new FontImportDialog(database, destination_dir, filenames, import_result);
		dlg.set_transient_for(parent_window);
		dlg.set_modal(true);
		dlg.show_all();
	}
}

} /* namespace Crown */
