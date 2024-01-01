/*
 * Copyright (c) 2012-2024 Daniele Bartolini et al.
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
private void copy_alpha_to_argb32(Cairo.ImageSurface dst, Cairo.ImageSurface src)
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

public class FontImportDialog : Gtk.Dialog
{
	public Cairo.Surface _checker;
	public FontAtlas* _font_atlas;
	public Cairo.ImageSurface _atlas;
	public Gtk.DrawingArea _drawing_area;
	public Gtk.ScrolledWindow _scrolled_window;

	public Gtk.Label _atlas_size;
	public Gtk.Label _font_path;
	public EntryResourceBasename _font_name;
	public EntryDouble _font_size;
	public Gtk.ComboBoxText _font_chars;
	public EntryDouble _font_range_min;
	public EntryDouble _font_range_max;

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

	public FontImportDialog(string font_path, string font_name)
	{
		this.border_width = 4;
		this.title = "Import Font...";
		this.set_icon_name(CROWN_ICON_NAME);

		// Create checkered pattern
		{
			int width = 16;
			int height = 16;
			_checker = new Cairo.ImageSurface(Cairo.Format.RGB24, width, height);

			Cairo.Context cr = new Cairo.Context(_checker);
			cr.set_source_rgb(0.9, 0.9, 0.9);
			cr.paint();
			cr.set_source_rgb(0.7, 0.7, 0.7);
			cr.rectangle(width / 2, 0, width / 2, height / 2);
			cr.rectangle(0, height / 2, width / 2, height / 2);
			cr.fill();
		}

		_drawing_area = new Gtk.DrawingArea();
		_drawing_area.draw.connect((cr) => {
				cr.set_source_rgb(0.1, 0.1, 0.1);
				cr.paint();

				cr.save();
				cr.set_source_surface(_checker, 0, 0);
				Cairo.Pattern pattern = cr.get_source();
				pattern.set_filter(Cairo.Filter.NEAREST);
				pattern.set_extend(Cairo.Extend.REPEAT);
				cr.rectangle(0, 0, _atlas.get_width(), _atlas.get_height());
				cr.clip();
				cr.paint();
				cr.restore();

				cr.save();
				cr.set_source_surface(_atlas, 0, 0);
				cr.rectangle(0, 0, _atlas.get_width(), _atlas.get_height());
				cr.paint();
				cr.restore();

				return Gdk.EVENT_STOP;
			});

		_scrolled_window = new Gtk.ScrolledWindow(null, null);
		_scrolled_window.min_content_width = 640;
		_scrolled_window.min_content_height = 640;
		_scrolled_window.add(_drawing_area);

		_atlas_size = new Gtk.Label("? × ?");
		_atlas_size.halign = Gtk.Align.START;
		_font_path = new Gtk.Label(font_path);
		_font_name = new EntryResourceBasename(font_name);
		_font_size = new EntryDouble(24.0, 1.0, 999.0);
		_font_range_min = new EntryDouble(32.0, 0.0, int32.MAX);
		_font_range_min.sensitive = false;
		_font_range_max = new EntryDouble(126.0, 0.0, int32.MAX);
		_font_range_max.sensitive = false;
		_font_chars = new Gtk.ComboBoxText();
		_font_chars.append_text("ASCII Printable"); // FontChars.ASCII_PRINTABLE
		_font_chars.append_text("ASCII Numbers");   // FontChars.ASCII_NUMBERS
		_font_chars.append_text("ASCII Letters");   // FontChars.ASCII_LETTERS
		_font_chars.append_text("Custom Range");    // FontChars.CUSTOM_RANGE
		_font_chars.active = FontChars.ASCII_PRINTABLE;
		_font_chars.changed.connect(() => {
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
		sprite_set.border_width = 12;

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

		this.get_content_area().add(pane);
		this.add_button("Cancel", Gtk.ResponseType.CANCEL);
		this.add_button("OK", Gtk.ResponseType.OK);
		this.response.connect(on_response);
		this.destroy.connect(on_destroy);
		this.map_event.connect(on_map_event);
	}

	private bool on_map_event(Gdk.EventAny ev)
	{
		_font_name.grab_focus();
		return Gdk.EVENT_PROPAGATE;
	}

	private void on_response(Gtk.Dialog source, int response_id)
	{
		switch (response_id) {
		case Gtk.ResponseType.OK:
			break;

		case Gtk.ResponseType.CANCEL:
			destroy();
			break;
		}
	}

	private void on_destroy()
	{
		font_atlas_free(_font_atlas);
	}

	public void load(Hashtable importer_settings)
	{
		_font_chars.active = FontChars.CUSTOM_RANGE;
		_font_size.value = (double)importer_settings["size"];
		set_font_range((int)(double)importer_settings["range_min"], (int)(double)importer_settings["range_max"]);
	}

	public void save(Hashtable importer_settings)
	{
		importer_settings["size"]      = _font_size.value;
		importer_settings["range_min"] = _font_range_min.value;
		importer_settings["range_max"] = _font_range_max.value;
	}
}

} /* namespace Crown */
