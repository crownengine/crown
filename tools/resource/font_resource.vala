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

[Compact]
public class FontImportOptions
{
	public bool options_loaded;
	public string font_name;
	public double font_size;
	public int range_min;
	public int range_max;

	public FontImportOptions(string font_name)
	{
		options_loaded = false;
		this.font_name = font_name;
		font_size = 24.0;
		range_min = 32;
		range_max = 126;
	}

	public void decode(Hashtable obj)
	{
		font_size = (double)obj["size"];
		range_min = (int)(double)obj["range_min"];
		range_max = (int)(double)obj["range_max"];
		options_loaded = true;
	}

	public Hashtable encode()
	{
		Hashtable obj = new Hashtable();

		obj["size"]      = font_size;
		obj["range_min"] = (double)range_min;
		obj["range_max"] = (double)range_max;

		return obj;
	}
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

	public FontImportOptions _options;

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

	public FontImportDialog(Database database
		, string destination_dir
		, GLib.SList<string> filenames
		, Import import_result
		, owned FontImportOptions options
		)
	{
		this.set_icon_name(CROWN_EDITOR_ICON_NAME);

		_project = database._project;
		_destination_dir = destination_dir;
		_filenames = new GLib.SList<string>();
		foreach (var f in filenames)
			_filenames.append(f);
		_import_result = import_result;
		_options = (owned)options;

		string font_path;
		{
			GLib.File file_src = File.new_for_path(filenames.nth_data(0));
			font_path = file_src.get_path();
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
		_font_name = new InputResourceBasename(_options.font_name);
		_font_name.sensitive = filenames.length() == 1;
		_font_size = new InputDouble(_options.font_size, 1.0, 999.0);
		_font_range_min = new InputDouble(_options.range_min, 0.0, int32.MAX);
		_font_range_min.sensitive = false;
		_font_range_max = new InputDouble(_options.range_max, 0.0, int32.MAX);
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
		cv.add_row(_("Source"), _font_path, _("Source font path."));
		sprite_set.add_property_grid(cv, _("File"));

		cv = new PropertyGrid();
		cv.add_row(_("Name"), _font_name, _("Name of the imported resource."));
		cv.add_row(_("Atlas size"), _atlas_size, _("Resolution of the generated texture."));
		cv.add_row(_("Size"), _font_size, _("Size of characters."));
		cv.add_row(_("Charset"), _font_chars, _("Generate characters within this set."));
		cv.add_row(_("Range min"), _font_range_min, _("First character to generate (included)."));
		cv.add_row(_("Range max"), _font_range_max, _("Last character to generate (included)."));
		sprite_set.add_property_grid(cv, _("Font"));

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

		_cancel = new Gtk.Button.with_label(_("Cancel"));
		_cancel.clicked.connect(() => {
				close();
			});
		_import = new Gtk.Button.with_label(_("Import"));
		_import.get_style_context().add_class("suggested-action");
		_import.clicked.connect(on_import);

		_header_bar = new Gtk.HeaderBar();
		_header_bar.title = _("Import Font...");
		_header_bar.show_close_button = true;
		_header_bar.pack_start(_cancel);
		_header_bar.pack_end(_import);

		this.set_titlebar(_header_bar);
		this.add(_box);

		if (_options.options_loaded) {
			_font_chars.active = FontChars.CUSTOM_RANGE;
			_font_range_min.sensitive = true;
			_font_range_max.sensitive = true;
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

	public void read_options()
	{
		_options.font_name = _font_name.value;
		_options.font_size = _font_size.value;
		_options.range_min = (int)_font_range_min.value;
		_options.range_max = (int)_font_range_max.value;
	}

	public void on_import()
	{
		read_options();
		FontResource.import_with_options(_import_result, _options, _project, _destination_dir, _filenames);
		close();
	}
}

public class FontResource
{
	public static string resource_basename(FontImportOptions options, GLib.SList<string> filenames, string filename)
	{
		GLib.File file_src = File.new_for_path(filename);
		if (filenames.length() == 1) {
			string? font_type = ResourceId.type(filename);
			return font_type == null
				? options.font_name
				: options.font_name + "." + font_type
				;
		} else {
			return file_src.get_basename();
		}
	}

	public static string? primary_resource_path(FontImportOptions options, Project project, string destination_dir, GLib.SList<string> filenames, ImportResult result)
	{
		if (result != ImportResult.SUCCESS || filenames.length() == 0)
			return null;

		string resource_basename = FontResource.resource_basename(options, filenames, filenames.nth_data(0));
		GLib.File file_dst       = File.new_for_path(Path.build_filename(destination_dir, resource_basename));
		string resource_filename = project.resource_filename(file_dst.get_path());
		string resource_path     = ResourceId.normalize(resource_filename);
		string resource_name     = ResourceId.name(resource_path);
		return ResourceId.path(OBJECT_TYPE_FONT, resource_name);
	}

	public static void import_with_options(Import import_result
		, FontImportOptions options
		, Project project
		, string destination_dir
		, GLib.SList<string> filenames
		)
	{
		ImportResult result = FontResource.do_import(options, project, destination_dir, filenames);
		import_result(result, FontResource.primary_resource_path(options, project, destination_dir, filenames, result));
	}

	public static ImportResult do_import(FontImportOptions options, Project project, string destination_dir, GLib.SList<string> filenames)
	{
		int font_size = (int)options.font_size;

		foreach (unowned string filename_i in filenames) {
			GLib.File file_src = File.new_for_path(filename_i);
			FontAtlas* font_atlas = (FontAtlas*)font_atlas_generate(file_src.get_path()
				, font_size
				, options.range_min
				, options.range_max
				);
			if (font_atlas == null)
				return ImportResult.ERROR;

			int size = font_atlas.size;
			string resource_basename = FontResource.resource_basename(options, filenames, filename_i);

			GLib.File file_dst       = File.new_for_path(Path.build_filename(destination_dir, resource_basename));
			string resource_filename = project.resource_filename(file_dst.get_path());
			string resource_path     = ResourceId.normalize(resource_filename);
			string resource_name     = ResourceId.name(resource_path);

			try {
				SJSON.save(options.encode(), project.absolute_path(resource_name) + ".importer_settings");
			} catch (JsonWriteError e) {
				return ImportResult.ERROR;
			}

			// Save .png atlas.
			var atlas = new Cairo.ImageSurface.for_data((uchar[])font_atlas.image_data
				, Cairo.Format.A8
				, font_atlas.size
				, font_atlas.size
				, Cairo.Format.A8.stride_for_width(font_atlas.size)
				);
			var argb32 = new Cairo.ImageSurface(Cairo.Format.ARGB32
				, atlas.get_width()
				, atlas.get_height()
				);
			copy_alpha_to_argb32(argb32, atlas);
			argb32.write_to_png(project.absolute_path(resource_name) + ".png");

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

			for (int ii = 0; ii < options.range_max - options.range_min + 1; ++ii) {
				GlyphData* gd = &font_atlas.glyphs[ii];

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

		return ImportResult.SUCCESS;
	}

	public static void import(Import import_result, Database database, string destination_dir, SList<string> filenames, Gtk.Window? parent_window)
	{
		if (filenames.length() == 0) {
			import_result(ImportResult.ERROR);
			return;
		}

		Project project = database._project;

		GLib.File file_src       = File.new_for_path(filenames.nth_data(0));
		GLib.File file_dst       = File.new_for_path(Path.build_filename(destination_dir, file_src.get_basename()));
		string resource_filename = project.resource_filename(file_dst.get_path());
		string resource_path     = ResourceId.normalize(resource_filename);
		string resource_name     = ResourceId.name(resource_path);
		int last_slash = resource_name.last_index_of_char('/');
		string font_name = last_slash == -1
			? resource_name
			: resource_name.substring(last_slash + 1, resource_name.length - last_slash - 1)
			;
		FontImportOptions options = new FontImportOptions(font_name);
		string settings_path     = project.absolute_path(resource_name) + ".importer_settings";
		if (File.new_for_path(settings_path).query_exists()) {
			try {
				options.decode(SJSON.load_from_path(settings_path));
			} catch (JsonSyntaxError e) {
				// No-op.
			}
		}

		if (parent_window == null) {
			FontResource.import_with_options(import_result
				, options
				, project
				, destination_dir
				, filenames
				);
		} else {
			FontImportDialog dlg = new FontImportDialog(database, destination_dir, filenames, import_result, (owned)options);
			dlg.set_transient_for(parent_window);
			dlg.set_modal(true);
			dlg.show_all();
		}
	}
}

} /* namespace Crown */
