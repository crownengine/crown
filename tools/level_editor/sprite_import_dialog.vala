/*
 * Copyright (c) 2012-2021 Daniele Bartolini et al.
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

public class SpriteImportDialog : Gtk.Dialog
{
	public Cairo.Surface _checker;
	public Gdk.Pixbuf _pixbuf;
	public Gtk.DrawingArea _drawing_area;
	public Gtk.ScrolledWindow _scrolled_window;
	public Gtk.DrawingArea _preview;

	public Gtk.Label resolution;
	public EntryVector2 cells;
	public Gtk.CheckButton cell_wh_auto;
	public EntryVector2 cell;
	public EntryVector2 offset;
	public EntryVector2 spacing;
	public Gtk.ComboBoxText pivot;
	public EntryDouble layer;
	public EntryDouble depth;

	public Gtk.CheckButton collision_enabled;
	public string shape_active_name;
	public Gtk.StackSwitcher shape_switcher;
	public Gtk.Stack shape;

	public EntryVector2 circle_collision_center;
	public EntryDouble circle_collision_radius;

	public EntryVector2 capsule_collision_center;
	public EntryDouble capsule_collision_height;
	public EntryDouble capsule_collision_radius;

	public EntryVector2 collision_xy;
	public EntryVector2 collision_wh;
	public ComboBoxMap actor_class;
	public EntryDouble mass;
	public Gtk.CheckButton lock_rotation_y;

	// Widgets
	public SpriteImportDialog(string png)
	{
		this.border_width = 4;
		this.title = "Import Sprite...";

		try
		{
			_pixbuf = new Gdk.Pixbuf.from_file(png);
		}
		catch (GLib.Error e)
		{
			loge(e.message);
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
			Vector2 pivot = sprite_cell_pivot_xy((int)cell.value.x
				, (int)cell.value.y
				, (int)pivot.active
				);

			int num_v = (int)cells.value.y;
			int num_h = (int)cells.value.x;

			for (int h = 0; h < num_v; ++h)
			{
				for (int w = 0; w < num_h; ++w)
				{
					Vector2 sc = sprite_cell_xy(h
						, w
						, (int)offset.value.x
						, (int)offset.value.y
						, (int)cell.value.x
						, (int)cell.value.y
						, (int)spacing.value.x
						, (int)spacing.value.y
						);

					int x0 = (int)sc.x;
					int y0 = (int)sc.y;
					int x1 = x0+(int)cell.value.x;
					int y1 = y0;
					int x2 = x1;
					int y2 = y0+(int)cell.value.y;
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

			return Gdk.EVENT_STOP;
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

			Vector2 sc = sprite_cell_xy(0
				, 0
				, (int)offset.value.x
				, (int)offset.value.y
				, (int)cell.value.x
				, (int)cell.value.y
				, (int)spacing.value.x
				, (int)spacing.value.y
				);

			int x0 = (int)sc.x;
			int y0 = (int)sc.y;
			int x1 = x0+(int)cell.value.x;
			int y2 = y0+(int)cell.value.y;

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
			if (shape.visible_child_name == "square_collider") {
				cr.rectangle(collision_xy.value.x, collision_xy.value.y, collision_wh.value.x, collision_wh.value.y);
				cr.set_source_rgba(0.3, 0.3, 0.3, 0.6);
				cr.fill();
			} else if (shape.visible_child_name == "circle_collider") {
				cr.arc(circle_collision_center.value.x, circle_collision_center.value.y, circle_collision_radius.value, 0, 2*Math.PI);
				cr.set_source_rgba(0.3, 0.3, 0.3, 0.6);
				cr.fill();
			} else if (shape.visible_child_name == "capsule_collider") {
				double x = capsule_collision_center.value.x;
				double y = capsule_collision_center.value.y;
				double radius = capsule_collision_radius.value;
				double height = capsule_collision_height.value - 2*radius;
				cr.arc(x - height/2, y, radius, Math.PI/2, 3*Math.PI/2);
				cr.rectangle(x - height/2, y - radius, height, 2*radius);
				cr.arc(x + height/2, y, radius, 3*Math.PI/2, Math.PI/2);
				cr.set_source_rgba(0.3, 0.3, 0.3, 0.6);
				cr.fill();
			}

			return Gdk.EVENT_STOP;
		});

		resolution = new Gtk.Label(_pixbuf.width.to_string() + " × " + _pixbuf.height.to_string());
		resolution.halign = Gtk.Align.START;

		cells = new EntryVector2(Vector2(4.0, 4.0), Vector2(1.0, 1.0), Vector2(256.0, 256.0));
		cell_wh_auto = new Gtk.CheckButton();
		cell_wh_auto.active = true;
		cell = new EntryVector2(Vector2(_pixbuf.width / cells.value.x, _pixbuf.height / cells.value.y), Vector2(1.0, 1.0), Vector2(double.MAX, double.MAX));
		cell.sensitive = !cell_wh_auto.active;
		offset = new EntryVector2(Vector2(0.0, 0.0), Vector2(0.0, 0.0), Vector2(double.MAX, double.MAX));
		spacing = new EntryVector2(Vector2(0.0, 0.0), Vector2(0.0, 0.0), Vector2(double.MAX, double.MAX));

		collision_enabled = new Gtk.CheckButton();
		collision_enabled.active = true;
		collision_xy = new EntryVector2(Vector2(0.0, 0.0), Vector2(-double.MAX, -double.MAX), Vector2(double.MAX, double.MAX));
		collision_wh = new EntryVector2(Vector2(32.0, 32.0), Vector2(-double.MAX, -double.MAX), Vector2(double.MAX, double.MAX));
		actor_class = new ComboBoxMap();
		actor_class.append("static", "static");
		actor_class.append("dynamic", "dynamic");
		actor_class.append("keyframed", "keyframed");
		actor_class.append("trigger", "trigger");
		actor_class.value = "static";
		lock_rotation_y = new Gtk.CheckButton();
		lock_rotation_y.active = true;
		mass = new EntryDouble(10.0, 0.0, double.MAX);

		circle_collision_center = new EntryVector2(Vector2(cell.value.x/2.0, cell.value.y/2.0), Vector2(-double.MAX, -double.MAX), Vector2(double.MAX, double.MAX));
		circle_collision_radius = new EntryDouble(32.0, 0.0, double.MAX);

		capsule_collision_center = new EntryVector2(Vector2(cell.value.x/2.0, cell.value.y/2.0), Vector2(-double.MAX, -double.MAX), Vector2(double.MAX, double.MAX));
		capsule_collision_radius = new EntryDouble(32.0, -double.MAX, double.MAX);
		capsule_collision_height = new EntryDouble(64.0, 0.0, double.MAX);

		cells.value_changed.connect (() => {
			if (cell_wh_auto.active)
				cell.value = Vector2(_pixbuf.width / cells.value.x, _pixbuf.height / cells.value.y);

			_drawing_area.queue_draw();
			_preview.queue_draw();
		});

		cell_wh_auto.toggled.connect(() => {
			cell.sensitive = !cell_wh_auto.active;
			cell.value = Vector2(_pixbuf.width / cells.value.x, _pixbuf.height / cells.value.y);

			_drawing_area.queue_draw();
			_preview.queue_draw();
		});

		cell.value_changed.connect (() => {
			circle_collision_center.value = Vector2(cell.value.x/2.0, cell.value.y/2.0);
			capsule_collision_center.value = Vector2(cell.value.x/2.0, cell.value.y/2.0);
			_drawing_area.queue_draw();
			_preview.queue_draw();
		});

		offset.value_changed.connect(() => {
			_drawing_area.queue_draw();
			_preview.queue_draw();
		});

		spacing.value_changed.connect(() => {
			_drawing_area.queue_draw();
			_preview.queue_draw();
		});

		collision_enabled.toggled.connect(() => {
			collision_xy.sensitive = !collision_xy.sensitive;
			collision_wh.sensitive = !collision_wh.sensitive;
			shape_switcher.sensitive = !shape_switcher.sensitive;
			circle_collision_center.sensitive = !circle_collision_center.sensitive;
			circle_collision_radius.sensitive = !circle_collision_radius.sensitive;
			capsule_collision_center.sensitive = !capsule_collision_center.sensitive;
			capsule_collision_radius.sensitive = !capsule_collision_radius.sensitive;
			capsule_collision_height.sensitive = !capsule_collision_height.sensitive;
			actor_class.sensitive = !actor_class.sensitive;
			mass.sensitive = !mass.sensitive;
			lock_rotation_y.sensitive = !lock_rotation_y.sensitive;
		});

		collision_xy.value_changed.connect(() => {
			_preview.queue_draw();
		});

		collision_wh.value_changed.connect(() => {
			_preview.queue_draw();
		});

		circle_collision_center.value_changed.connect(() => {
			_preview.queue_draw();
		});

		circle_collision_radius.value_changed.connect(() => {
			_preview.queue_draw();
		});

		capsule_collision_center.value_changed.connect(() => {
			_preview.queue_draw();
		});

		capsule_collision_radius.value_changed.connect(() => {
			_preview.queue_draw();
		});

		capsule_collision_height.value_changed.connect(() => {
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

		layer = new EntryDouble(0.0, 0.0, 7.0);
		depth = new EntryDouble(0.0, 0.0, 9999.0);

		PropertyGridSet sprite_set = new PropertyGridSet();
		sprite_set.border_width = 6;

		PropertyGrid cv;
		cv = new PropertyGrid();
		cv.add_row("Resolution", resolution);
		cv.add_row("Cells", cells);
		cv.add_row("Auto Size", cell_wh_auto);
		cv.add_row("Cell", cell);
		cv.add_row("Offset", offset);
		cv.add_row("Spacing", spacing);
		cv.add_row("Pivot", pivot);
		sprite_set.add_property_grid(cv, "Image");

		cv = new PropertyGrid();
		cv.add_row("Layer", layer);
		cv.add_row("Depth", depth);
		sprite_set.add_property_grid(cv, "Sprite Renderer");

		cv = new PropertyGrid();
		cv.add_row("Collision", collision_enabled);
		cv.add_row("Class", actor_class);
		cv.add_row("Mass", mass);
		cv.add_row("Lock Rotation", lock_rotation_y);
		sprite_set.add_property_grid(cv, "Actor");

		shape = new Gtk.Stack();
		shape.notify["visible-child"].connect(() => { _preview.queue_draw(); });

		cv = new PropertyGrid();
		cv.add_row("Origin", collision_xy);
		cv.add_row("Size", collision_wh);
		shape.add_titled(cv, "square_collider", "Square");

		cv = new PropertyGrid();
		cv.add_row("Origin", circle_collision_center);
		cv.add_row("Radius", circle_collision_radius);
		shape.add_titled(cv, "circle_collider", "Circle");

		cv = new PropertyGrid();
		cv.add_row("Origin", capsule_collision_center);
		cv.add_row("Radius", capsule_collision_radius);
		cv.add_row("Height", capsule_collision_height);
		shape.add_titled(cv, "capsule_collider", "Capsule");

		shape_switcher = new Gtk.StackSwitcher();
		shape_switcher.set_stack(shape);

		cv = new PropertyGrid();
		cv.add_row("Shape Type", shape_switcher);
		cv.add_row("Shape Data", shape);
		sprite_set.add_property_grid(cv, "Collider");

		Gtk.Box box = new Gtk.Box(Gtk.Orientation.HORIZONTAL, 0);
		box.pack_start(_scrolled_window, true, true);
		box.pack_start(_preview, true, true);

		Gtk.Paned pane;
		pane = new Gtk.Paned(Gtk.Orientation.HORIZONTAL);
		pane.pack1(box, false, false);
		pane.pack2(sprite_set, true, false);

		this.get_content_area().add(pane);
		this.add_button("Cancel", Gtk.ResponseType.CANCEL);
		this.add_button("OK", Gtk.ResponseType.OK);
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
		cells.value              = Vector2((double)importer_settings["num_h"], (double)importer_settings["num_v"]);
		cell.value               = Vector2((double)importer_settings["cell_w"], (double)importer_settings["cell_h"]);
		offset.value             = Vector2((double)importer_settings["offset_x"], (double)importer_settings["offset_y"]);
		spacing.value            = Vector2((double)importer_settings["spacing_x"], (double)importer_settings["spacing_y"]);
		layer.value              = (double)importer_settings["layer"];
		depth.value              = (double)importer_settings["depth"];
		pivot.active             = (int)(double)importer_settings["pivot"];
		collision_enabled.active = (bool)importer_settings["collision_enabled"];
		collision_xy.value       = Vector2((double)importer_settings["collision_x"], (double)importer_settings["collision_y"]);
		collision_wh.value       = Vector2((double)importer_settings["collision_w"], (double)importer_settings["collision_h"]);

		circle_collision_center.value = Vector2(importer_settings.has_key("circle_collision_center_x") ? (double)importer_settings["circle_collision_center_x"] : cell.value.x/2.0, importer_settings.has_key("circle_collision_center_y") ? (double)importer_settings["circle_collision_center_y"] : cell.value.y/2.0);
		circle_collision_radius.value = importer_settings.has_key("circle_collision_radius") ? (double)importer_settings["circle_collision_radius"] : 32;

		capsule_collision_center.value = Vector2(importer_settings.has_key("capsule_collision_center_x") ? (double)importer_settings["capsule_collision_center_x"] : cell.value.x/2.0, importer_settings.has_key("capsule_collision_center_y") ? (double)importer_settings["capsule_collision_center_y"] : cell.value.y/2.0);
		capsule_collision_radius.value = importer_settings.has_key("capsule_collision_radius") ? (double)importer_settings["capsule_collision_radius"] : 32;
		capsule_collision_height.value = importer_settings.has_key("capsule_collision_height") ? (double)importer_settings["capsule_collision_height"] : 64;

		shape.visible_child_name = importer_settings.has_key("shape_active_name") ? (string)importer_settings["shape_active_name"] : "square_collider";
		actor_class.value        = importer_settings.has_key("actor_class") ? (string)importer_settings["actor_class"] : "static";
		lock_rotation_y.active   = importer_settings.has_key("lock_rotation_y") ? (bool)importer_settings["lock_rotation_y"] : true;
		mass.value               = importer_settings.has_key("mass") ? (double)importer_settings["mass"] : 10.0;
	}

	public void save(Hashtable importer_settings)
	{
		importer_settings["num_h"]                      = cells.value.x;
		importer_settings["num_v"]                      = cells.value.y;
		importer_settings["cell_w"]                     = cell.value.x;
		importer_settings["cell_h"]                     = cell.value.y;
		importer_settings["offset_x"]                   = offset.value.x;
		importer_settings["offset_y"]                   = offset.value.y;
		importer_settings["spacing_x"]                  = spacing.value.x;
		importer_settings["spacing_y"]                  = spacing.value.y;
		importer_settings["layer"]                      = layer.value;
		importer_settings["depth"]                      = depth.value;
		importer_settings["pivot"]                      = pivot.active;
		importer_settings["collision_enabled"]          = collision_enabled.active;
		importer_settings["collision_x"]                = collision_xy.value.x;
		importer_settings["collision_y"]                = collision_xy.value.y;
		importer_settings["collision_w"]                = collision_wh.value.x;
		importer_settings["collision_h"]                = collision_wh.value.y;
		importer_settings["circle_collision_center_x"]  = circle_collision_center.value.x;
		importer_settings["circle_collision_center_y"]  = circle_collision_center.value.y;
		importer_settings["circle_collision_radius"]    = circle_collision_radius.value;
		importer_settings["capsule_collision_center_x"] = capsule_collision_center.value.x;
		importer_settings["capsule_collision_center_y"] = capsule_collision_center.value.y;
		importer_settings["capsule_collision_radius"]   = capsule_collision_radius.value;
		importer_settings["capsule_collision_height"]   = capsule_collision_height.value;
		importer_settings["shape_active_name"]          = shape.visible_child_name;
		importer_settings["actor_class"]                = actor_class.value;
		importer_settings["lock_rotation_y"]            = lock_rotation_y.active;
		importer_settings["mass"]						= mass.value;
	}
}

}
