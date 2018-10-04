/*
 * Copyright (c) 2012-2018 Daniele Bartolini and individual contributors.
 * License: https://github.com/dbartolini/crown/blob/master/LICENSE
 */

using Cairo;
using Gdk;
using Gtk;

namespace Crown
{
public enum Pivot
{
	TOP_LEFT,
	TOP_CENTER,
	TOP_RIGHT,
	LEFT,
	CENTER,
	RIGHT,
	BOTTOM_LEFT,
	BOTTOM_CENTER,
	BOTTOM_RIGHT
}

Vector2 sprite_cell_xy(int r, int c, int offset_x, int offset_y, int cell_w, int cell_h, int spacing_x, int spacing_y)
{
	int x0 = offset_x + c*cell_w + c*spacing_x;
	int y0 = offset_y + r*cell_h + r*spacing_y;
	return Vector2(x0, y0);
}

Vector2 sprite_cell_pivot_xy(int cell_w, int cell_h, int pivot)
{
	int pivot_x = 0;
	int pivot_y = 0;

	switch (pivot)
	{
	case Pivot.TOP_LEFT:
		pivot_x = 0;
		pivot_y = 0;
		break;

	case Pivot.TOP_CENTER:
		pivot_x = cell_w / 2;
		pivot_y = 0;
		break;

	case Pivot.TOP_RIGHT:
		pivot_x = cell_w;
		pivot_y = 0;
		break;

	case Pivot.BOTTOM_LEFT:
		pivot_x = 0;
		pivot_y = cell_h;
		break;

	case Pivot.BOTTOM_CENTER:
		pivot_x = cell_w / 2;
		pivot_y = cell_h;
		break;

	case Pivot.BOTTOM_RIGHT:
		pivot_x = cell_w;
		pivot_y = cell_h;
		break;

	case Pivot.LEFT:
		pivot_x = 0;
		pivot_y = cell_h / 2;
		break;

	case Pivot.CENTER:
		pivot_x = cell_w / 2;
		pivot_y = cell_h / 2;
		break;

	case Pivot.RIGHT:
		pivot_x = cell_w;
		pivot_y = cell_h / 2;
		break;

	default:
		assert(false);
		break;
	}

	return Vector2(pivot_x, pivot_y);
}

Gtk.Label label_with_alignment(string text, Gtk.Align align)
{
	var l = new Label(text);
	l.halign = align;
	return l;
}

public class SpriteImportDialog : Gtk.Dialog
{
	public Cairo.Surface _checker;
	public Gdk.Pixbuf _pixbuf;
	public Gtk.DrawingArea _drawing_area;
	public Gtk.ScrolledWindow _scrolled_window;
	public Gtk.DrawingArea _preview;

	public Gtk.Label resolution;
	public Gtk.SpinButton cells_h;
	public Gtk.SpinButton cells_v;
	public Gtk.CheckButton cell_wh_auto;
	public Gtk.SpinButton cell_w;
	public Gtk.SpinButton cell_h;
	public Gtk.SpinButton offset_x;
	public Gtk.SpinButton offset_y;
	public Gtk.SpinButton spacing_x;
	public Gtk.SpinButton spacing_y;
	public Gtk.ComboBoxText pivot;
	public SpinButtonDouble layer;
	public SpinButtonDouble depth;

	public Gtk.CheckButton collision_enabled;
	public Gtk.SpinButton collision_x;
	public Gtk.SpinButton collision_y;
	public Gtk.SpinButton collision_w;
	public Gtk.SpinButton collision_h;

	// Widgets
	public SpriteImportDialog(string png)
	{
		this.border_width = 18;
		this.title = "Import Sprite...";

		try {
			_pixbuf = new Gdk.Pixbuf.from_file(png);
		} catch(GLib.Error err) {
			stdout.printf("Pixbuf.from_file: error");
		}

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
		_drawing_area.set_size_request(_pixbuf.width, _pixbuf.height);

		_drawing_area.draw.connect((cr) => {
				cr.set_source_rgb(0.1, 0.1, 0.1);
				cr.paint();

				cr.save();
				cr.set_source_surface(_checker, 0, 0);
				Cairo.Pattern pattern = cr.get_source();
				pattern.set_filter(Cairo.Filter.NEAREST);
				pattern.set_extend(Cairo.Extend.REPEAT);
				cr.rectangle(0, 0, _pixbuf.width, _pixbuf.height);
				cr.clip();
				cr.paint();
				cr.restore();

				Gdk.cairo_set_source_pixbuf(cr, _pixbuf, 0, 0);
				cr.paint();

				// Pivot is relative to the top-left corner of the cell
				Vector2 pivot = sprite_cell_pivot_xy((int)cell_w.value
					, (int)cell_h.value
					, (int)pivot.active
					);

				int num_v = (int)cells_v.value;
				int num_h = (int)cells_h.value;

				for (int h = 0; h < num_v; ++h)
				{
					for (int w = 0; w < num_h; ++w)
					{
						Vector2 cell = sprite_cell_xy(h
							, w
							, (int)offset_x.value
							, (int)offset_y.value
							, (int)cell_w.value
							, (int)cell_h.value
							, (int)spacing_x.value
							, (int)spacing_y.value
							);

						int x0 = (int)cell.x;
						int y0 = (int)cell.y;
						int x1 = x0+(int)cell_w.value;
						int y1 = y0;
						int x2 = x1;
						int y2 = y0+(int)cell_h.value;
						int x3 = x0;
						int y3 = y2;
						// https://www.cairographics.org/FAQ/#sharp_lines
						cr.move_to((double)x0,       (double)y0 + 0.5);
						cr.line_to((double)x1,       (double)y1 + 0.5);
						cr.move_to((double)x1 + 0.5, (double)y1      );
						cr.line_to((double)x2 + 0.5, (double)y2      );
						cr.move_to((double)x2,       (double)y2 + 0.5);
						cr.line_to((double)x3,       (double)y3 + 0.5);
						cr.move_to((double)x3 + 0.5, (double)y3      );
						cr.line_to((double)x0 + 0.5, (double)y0      );

						cr.set_line_width(1);
						cr.set_source_rgba(0.9, 0.1, 0.1, 0.9);
						cr.stroke();

						cr.arc(x0 + pivot.x, y0 + pivot.y, 5.0, 0, 2*Math.PI);
						cr.set_source_rgba(0.1, 0.1, 0.9, 0.6);
						cr.fill();
					}
				}

				return true;
			});

		_scrolled_window = new Gtk.ScrolledWindow(null, null);
		_scrolled_window.min_content_width = 640;
		_scrolled_window.min_content_height = 640;
		_scrolled_window.add(_drawing_area);

		_preview = new Gtk.DrawingArea();
		_preview.set_size_request(128, 128);

		_preview.draw.connect((cr) => {
				cr.set_source_rgb(0.1, 0.1, 0.1);
				cr.paint();

				Vector2 cell = sprite_cell_xy(0
					, 0
					, (int)offset_x.value
					, (int)offset_y.value
					, (int)cell_w.value
					, (int)cell_h.value
					, (int)spacing_x.value
					, (int)spacing_y.value
					);

				int x0 = (int)cell.x;
				int y0 = (int)cell.y;
				int x1 = x0+(int)cell_w.value;
				int y2 = y0+(int)cell_h.value;

				// Draw checkered background
				cr.save();
				cr.set_source_surface(_checker, 0, 0);
				Cairo.Pattern pattern = cr.get_source();
				pattern.set_filter(Cairo.Filter.NEAREST);
				pattern.set_extend(Cairo.Extend.REPEAT);
				cr.rectangle(x0, y0, x1, y2);
				cr.clip();
				cr.new_path(); // path not consumed by clip()
				cr.paint();
				cr.restore();

				// Draw sprite
				cr.save();
				Gdk.cairo_set_source_pixbuf(cr, _pixbuf, 0, 0);
				cr.rectangle(x0, y0, x1, y2);
				cr.clip();
				cr.new_path(); // path not consumed by clip()
				cr.paint();
				cr.restore();

				// Draw collision
				cr.rectangle(collision_x.value, collision_y.value, collision_w.value, collision_h.value);
				cr.set_source_rgba(0.3, 0.3, 0.3, 0.6);
				cr.fill();

				return true;
			});

		resolution = new Gtk.Label(_pixbuf.width.to_string() + " × " + _pixbuf.height.to_string());
		resolution.halign = Gtk.Align.START;

		cells_h = new Gtk.SpinButton.with_range(1.0, 256.0, 1.0);
		cells_h.value = 4;
		cells_v = new Gtk.SpinButton.with_range(1.0, 256.0, 1.0);
		cells_v.value = 4;
		cell_wh_auto = new Gtk.CheckButton();
		cell_wh_auto.active = true;
		cell_w = new Gtk.SpinButton.with_range(1.0, 4096.0, 1.0);
		cell_w.value = _pixbuf.width / cells_h.value;
		cell_w.sensitive = !cell_wh_auto.active;
		cell_h = new Gtk.SpinButton.with_range(1.0, 4096.0, 1.0);
		cell_h.value = _pixbuf.height / cells_v.value;
		cell_h.sensitive = !cell_wh_auto.active;
		offset_x = new Gtk.SpinButton.with_range(0, 128.0, 1.0);
		offset_y = new Gtk.SpinButton.with_range(0, 128.0, 1.0);
		spacing_x = new Gtk.SpinButton.with_range(0, 128.0, 1.0);
		spacing_y = new Gtk.SpinButton.with_range(0, 128.0, 1.0);

		collision_enabled = new Gtk.CheckButton();
		collision_enabled.active = true;
		collision_x = new Gtk.SpinButton.with_range(0.0, 256.0, 1.0);
		collision_x.value = 0;
		collision_y = new Gtk.SpinButton.with_range(0.0, 256.0, 1.0);
		collision_y.value = 0;
		collision_w = new Gtk.SpinButton.with_range(0.0, 256.0, 1.0);
		collision_w.value = 32;
		collision_h = new Gtk.SpinButton.with_range(0.0, 256.0, 1.0);
		collision_h.value = 32;

		cells_h.value_changed.connect (() => {
			if (cell_wh_auto.active)
			{
				cell_w.value = _pixbuf.width / cells_h.value;
				cell_h.value = _pixbuf.height / cells_v.value;
			}
			_drawing_area.queue_draw();
			_preview.queue_draw();
		});

		cells_v.value_changed.connect(() => {
			if (cell_wh_auto.active)
			{
				cell_w.value = _pixbuf.width / cells_h.value;
				cell_h.value = _pixbuf.height / cells_v.value;
			}
			_drawing_area.queue_draw();
			_preview.queue_draw();
		});

		cell_wh_auto.toggled.connect(() => {
			cell_w.sensitive = !cell_wh_auto.active;
			cell_h.sensitive = !cell_wh_auto.active;
			cell_w.value = _pixbuf.width / cells_h.value;
			cell_h.value = _pixbuf.height / cells_v.value;
			_drawing_area.queue_draw();
			_preview.queue_draw();
		});

		cell_w.value_changed.connect (() => {
			_drawing_area.queue_draw();
			_preview.queue_draw();
		});

		cell_h.value_changed.connect(() => {
			_drawing_area.queue_draw();
			_preview.queue_draw();
		});

		offset_x.value_changed.connect(() => {
			_drawing_area.queue_draw();
			_preview.queue_draw();
		});

		offset_y.value_changed.connect(() => {
			_drawing_area.queue_draw();
			_preview.queue_draw();
		});

		spacing_x.value_changed.connect(() => {
			_drawing_area.queue_draw();
			_preview.queue_draw();
		});

		spacing_y.value_changed.connect(() => {
			_drawing_area.queue_draw();
			_preview.queue_draw();
		});

		collision_enabled.toggled.connect(() => {
			collision_x.sensitive = !collision_x.sensitive;
			collision_y.sensitive = !collision_y.sensitive;
			collision_w.sensitive = !collision_w.sensitive;
			collision_h.sensitive = !collision_h.sensitive;
		});

		collision_x.value_changed.connect(() => {
			_preview.queue_draw();
		});

		collision_y.value_changed.connect(() => {
			_preview.queue_draw();
		});

		collision_w.value_changed.connect(() => {
			_preview.queue_draw();
		});

		collision_h.value_changed.connect(() => {
			_preview.queue_draw();
		});

		pivot = new Gtk.ComboBoxText();
		pivot.append_text("Top left");      // TOP_LEFT
		pivot.append_text("Top center");    // TOP_CENTER
		pivot.append_text("Top right");     // TOP_RIGHT
		pivot.append_text("Left");          // LEFT
		pivot.append_text("Center");        // CENTER
		pivot.append_text("Right");         // RIGHT
		pivot.append_text("Bottom left");   // BOTTOM_LEFT
		pivot.append_text("Bottom center"); // BOTTOM_CENTER
		pivot.append_text("Bottom right");  // BOTTOM_RIGHT
		pivot.active = Pivot.CENTER;

		pivot.changed.connect(() => {
			_drawing_area.queue_draw();
			_preview.queue_draw();
		});

		layer = new SpinButtonDouble(0.0, 0.0, 7.0);
		depth = new SpinButtonDouble(0.0, 0.0, 9999.0);

		Gtk.Grid grid = new Gtk.Grid();
		grid.attach(label_with_alignment("Resolution", Gtk.Align.END),   0,  0, 1, 1);
		grid.attach(label_with_alignment("Cells H", Gtk.Align.END),      0,  1, 1, 1);
		grid.attach(label_with_alignment("Cells V", Gtk.Align.END),      0,  2, 1, 1);
		grid.attach(label_with_alignment("Cell WH auto", Gtk.Align.END), 0,  3, 1, 1);
		grid.attach(label_with_alignment("Cell W", Gtk.Align.END),       0,  4, 1, 1);
		grid.attach(label_with_alignment("Cell H", Gtk.Align.END),       0,  5, 1, 1);
		grid.attach(label_with_alignment("Offset X", Gtk.Align.END),     0,  6, 1, 1);
		grid.attach(label_with_alignment("Offset Y", Gtk.Align.END),     0,  7, 1, 1);
		grid.attach(label_with_alignment("Spacing X", Gtk.Align.END),    0,  8, 1, 1);
		grid.attach(label_with_alignment("Spacing Y", Gtk.Align.END),    0,  9, 1, 1);
		grid.attach(label_with_alignment("Pivot", Gtk.Align.END),        0, 10, 1, 1);
		grid.attach(label_with_alignment("Layer", Gtk.Align.END),        0, 11, 1, 1);
		grid.attach(label_with_alignment("Depth", Gtk.Align.END),        0, 12, 1, 1);

		grid.attach(label_with_alignment("Collision", Gtk.Align.END),    0, 13, 1, 1);
		grid.attach(label_with_alignment("Collision X", Gtk.Align.END),  0, 14, 1, 1);
		grid.attach(label_with_alignment("Collision Y", Gtk.Align.END),  0, 15, 1, 1);
		grid.attach(label_with_alignment("Collision W", Gtk.Align.END),  0, 16, 1, 1);
		grid.attach(label_with_alignment("Collision H", Gtk.Align.END),  0, 17, 1, 1);

		grid.attach(resolution,   1,  0, 1, 1);
		grid.attach(cells_h,      1,  1, 1, 1);
		grid.attach(cells_v,      1,  2, 1, 1);
		grid.attach(cell_wh_auto, 1,  3, 1, 1);
		grid.attach(cell_w,       1,  4, 1, 1);
		grid.attach(cell_h,       1,  5, 1, 1);
		grid.attach(offset_x,     1,  6, 1, 1);
		grid.attach(offset_y,     1,  7, 1, 1);
		grid.attach(spacing_x,    1,  8, 1, 1);
		grid.attach(spacing_y,    1,  9, 1, 1);
		grid.attach(pivot,        1, 10, 1, 1);
		grid.attach(layer,        1, 11, 1, 1);
		grid.attach(depth,        1, 12, 1, 1);

		grid.attach(collision_enabled, 1, 13, 1, 1);
		grid.attach(collision_x,       1, 14, 1, 1);
		grid.attach(collision_y,       1, 15, 1, 1);
		grid.attach(collision_w,       1, 16, 1, 1);
		grid.attach(collision_h,       1, 17, 1, 1);

		grid.row_spacing = 6;
		grid.column_spacing = 12;

		Gtk.Box box = new Gtk.Box(Gtk.Orientation.HORIZONTAL, 12);
		box.pack_start(_scrolled_window, true, true);
		box.pack_start(_preview, true, true);
		box.pack_end(grid, false, false);
		box.margin_bottom = 18;

		get_content_area().pack_start(box);
		get_content_area().show_all();

		add_button("Cancel", Gtk.ResponseType.CANCEL);
		add_button("OK", Gtk.ResponseType.OK);

		this.response.connect(on_response);
	}

	private void on_response(Gtk.Dialog source, int response_id)
	{
		switch (response_id)
		{
		case Gtk.ResponseType.OK:
			break;

		case Gtk.ResponseType.CANCEL:
			destroy();
			break;
		}
	}

	public void load(Hashtable importer_settings)
	{
		// Load settings
		cells_h.value            = (double)importer_settings["num_h"];
		cells_v.value            = (double)importer_settings["num_v"];
		cell_w.value             = (double)importer_settings["cell_w"];
		cell_h.value             = (double)importer_settings["cell_h"];
		offset_x.value           = (double)importer_settings["offset_x"];
		offset_y.value           = (double)importer_settings["offset_y"];
		spacing_x.value          = (double)importer_settings["spacing_x"];
		spacing_y.value          = (double)importer_settings["spacing_y"];
		layer.value              = (double)importer_settings["layer"];
		depth.value              = (double)importer_settings["depth"];
		pivot.active             = (int)(double)importer_settings["pivot"];
		collision_enabled.active = (bool)importer_settings["collision_enabled"];
		collision_x.value        = (double)importer_settings["collision_x"];
		collision_y.value        = (double)importer_settings["collision_y"];
		collision_w.value        = (double)importer_settings["collision_w"];
		collision_h.value        = (double)importer_settings["collision_h"];
	}

	public void save(Hashtable importer_settings)
	{
		importer_settings["num_h"]             = cells_h.value;
		importer_settings["num_v"]             = cells_v.value;
		importer_settings["cell_w"]            = cell_w.value;
		importer_settings["cell_h"]            = cell_h.value;
		importer_settings["offset_x"]          = offset_x.value;
		importer_settings["offset_y"]          = offset_y.value;
		importer_settings["spacing_x"]         = spacing_x.value;
		importer_settings["spacing_y"]         = spacing_y.value;
		importer_settings["layer"]             = layer.value;
		importer_settings["depth"]             = depth.value;
		importer_settings["pivot"]             = pivot.active;
		importer_settings["collision_enabled"] = collision_enabled.active;
		importer_settings["collision_x"]       = collision_x.value;
		importer_settings["collision_y"]       = collision_y.value;
		importer_settings["collision_w"]       = collision_w.value;
		importer_settings["collision_h"]       = collision_h.value;
	}
}

}
