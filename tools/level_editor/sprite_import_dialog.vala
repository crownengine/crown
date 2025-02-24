/*
 * Copyright (c) 2012-2025 Daniele Bartolini et al.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

namespace Crown
{
public Gdk.RGBA collider_color = { 1.0, 0.5, 0.0, 1.0 };

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

	switch (pivot) {
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

void sprite_cell_from_index(out int r, out int c, int num_cols, int index)
{
	r = (int)(index / num_cols);
	c = index - r * num_cols;
}

public class SpriteImportDialog : Gtk.Dialog
{
	public Gdk.Pixbuf _pixbuf;

	public EntryResourceBasename _unit_name;
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

	public Gtk.Button _previous_frame;
	public Gtk.Button _next_frame;
	public EntryDouble _current_frame;
	public Gtk.Box _frame_selector_box;
	public Gtk.Overlay _preview_overlay;
	public PixbufView _slices;
	public PixbufView _preview;
	public Gtk.ScrolledWindow _scrolled_window;

	public Gtk.Notebook _notebook;

	// Widgets
	public SpriteImportDialog(string image_path, string unit_name)
	{
		this.border_width = 4;
		this.title = "Import Sprite...";
		this.set_icon_name(CROWN_EDITOR_ICON_NAME);

		try {
			_pixbuf = new Gdk.Pixbuf.from_file(image_path);
		} catch (GLib.Error e) {
			loge(e.message);
		}

		_unit_name = new EntryResourceBasename(unit_name);

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
		circle_collision_radius = new EntryDouble(32.0, 0.5, double.MAX);

		capsule_collision_center = new EntryVector2(Vector2(cell.value.x/2.0, cell.value.y/2.0), Vector2(-double.MAX, -double.MAX), Vector2(double.MAX, double.MAX));
		capsule_collision_radius = new EntryDouble(32.0, 0.5, double.MAX);
		capsule_collision_height = new EntryDouble(64.0, 2.0*capsule_collision_radius.value, double.MAX);

		cells.value_changed.connect(() => {
				if (cell_wh_auto.active)
					cell.value = Vector2(_pixbuf.width / cells.value.x, _pixbuf.height / cells.value.y);

				_current_frame.set_max(cells.value.x * cells.value.y - 1);

				_slices.queue_draw();
				set_preview_frame();
				_preview.queue_draw();
			});

		cell_wh_auto.toggled.connect(() => {
				cell.sensitive = !cell_wh_auto.active;
				cell.value = Vector2(_pixbuf.width / cells.value.x, _pixbuf.height / cells.value.y);
				_slices.queue_draw();
				_preview.queue_draw();
			});

		cell.value_changed.connect(() => {
				circle_collision_center.value = Vector2(cell.value.x/2.0, cell.value.y/2.0);
				capsule_collision_center.value = Vector2(cell.value.x/2.0, cell.value.y/2.0);
				_slices.queue_draw();
				set_preview_frame();
				_preview.queue_draw();
			});

		offset.value_changed.connect(() => {
				_slices.queue_draw();
				set_preview_frame();
				_preview.queue_draw();
			});

		spacing.value_changed.connect(() => {
				_slices.queue_draw();
				set_preview_frame();
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
				capsule_collision_height.set_min(2.0 * capsule_collision_radius.value);
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
				_slices.queue_draw();
				_preview.queue_draw();
			});

		layer = new EntryDouble(0.0, 0.0, 7.0);
		depth = new EntryDouble(0.0, 0.0, 9999.0);

		PropertyGridSet sprite_set = new PropertyGridSet();
		sprite_set.border_width = 6;

		// Slices.
		PropertyGrid cv;
		cv = new PropertyGrid();
		cv.add_row("Name", _unit_name);
		sprite_set.add_property_grid(cv, "Unit");

		cv = new PropertyGrid();
		cv.add_row("Resolution", resolution);
		cv.add_row("Cells", cells);
		cv.add_row("Auto Size", cell_wh_auto);
		cv.add_row("Cell", cell);
		cv.add_row("Offset", offset);
		cv.add_row("Spacing", spacing);
		cv.add_row("Pivot", pivot);
		cv.add_row("Collision", collision_enabled);
		sprite_set.add_property_grid(cv, "Image");

		// Sprite Renderer.
		cv = new PropertyGrid();
		cv.add_row("Layer", layer);
		cv.add_row("Depth", depth);
		sprite_set.add_property_grid(cv, "Sprite Renderer");

		// Collider.
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

		// Actor.
		cv = new PropertyGrid();
		cv.add_row("Class", actor_class);
		cv.add_row("Mass", mass);
		cv.add_row("Lock Rotation", lock_rotation_y);
		sprite_set.add_property_grid(cv, "Actor");

		_previous_frame = new Gtk.Button.from_icon_name("go-previous-symbolic", Gtk.IconSize.LARGE_TOOLBAR);
		_previous_frame.clicked.connect(() => {
				_current_frame.value -= 1;
				set_preview_frame();
				_preview.queue_draw();
			});
		_next_frame = new Gtk.Button.from_icon_name("go-next-symbolic", Gtk.IconSize.LARGE_TOOLBAR);
		_next_frame.clicked.connect(() => {
				_current_frame.value += 1;
				set_preview_frame();
				_preview.queue_draw();
			});
		_current_frame = new EntryDouble(0.0, 0.0, cells.value.x * cells.value.y - 1);
		_current_frame.value_changed.connect(() => {
				set_preview_frame();
				_preview.queue_draw();
			});
		_frame_selector_box = new Gtk.Box(Gtk.Orientation.HORIZONTAL, 0);
		_frame_selector_box.homogeneous = true;
		_frame_selector_box.halign = Gtk.Align.CENTER;
		_frame_selector_box.valign = Gtk.Align.END;
		_frame_selector_box.margin_bottom = 24;
		_frame_selector_box.pack_start(_previous_frame);
		_frame_selector_box.pack_start(_current_frame);
		_frame_selector_box.pack_end(_next_frame);

		_slices = new PixbufView();
		_slices.set_size_request(_pixbuf.width, _pixbuf.height);
		_slices.set_pixbuf(_pixbuf);

		_slices.draw.connect((cr) => {
				int allocated_width = _preview.get_allocated_width();
				int allocated_height = _preview.get_allocated_height();
				double original_line_width = cr.get_line_width();

				cr.translate(allocated_width * 0.5, allocated_height * 0.5);
				cr.scale(_slices._zoom, _slices._zoom);
				cr.set_line_width(original_line_width / _slices._zoom);
				cr.translate(-_pixbuf.width * 0.5, -_pixbuf.height * 0.5);

				int num_v = (int)cells.value.y;
				int num_h = (int)cells.value.x;

				for (int h = 0; h < num_v; ++h) {
					for (int w = 0; w < num_h; ++w) {
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
						int x1 = x0 + (int)cell.value.x;
						int y1 = y0;
						int x2 = x1;
						int y2 = y0 + (int)cell.value.y;
						int x3 = x0;
						int y3 = y2;
						// https://www.cairographics.org/FAQ/#sharp_lines
						cr.move_to((double)x0,       (double)y0 + 0.5);
						cr.line_to((double)x1,       (double)y1 + 0.5);
						cr.move_to((double)x1 + 0.5, (double)y1);
						cr.line_to((double)x2 + 0.5, (double)y2);
						cr.move_to((double)x2,       (double)y2 + 0.5);
						cr.line_to((double)x3,       (double)y3 + 0.5);
						cr.move_to((double)x3 + 0.5, (double)y3);
						cr.line_to((double)x0 + 0.5, (double)y0);

						cr.set_source_rgba(0.9, 0.1, 0.1, 0.9);
						cr.stroke();
					}
				}

				return Gdk.EVENT_STOP;
			});

		_preview = new PixbufView();
		_preview._zoom = 4.0;
		_preview.set_size_request(128, 128);
		set_preview_frame();

		_preview.draw.connect((cr) => {
				int allocated_width = _preview.get_allocated_width();
				int allocated_height = _preview.get_allocated_height();
				double original_line_width = cr.get_line_width();

				cr.translate(allocated_width * 0.5, allocated_height * 0.5);
				cr.scale(_preview._zoom, _preview._zoom);
				cr.set_line_width(original_line_width / _preview._zoom);
				cr.translate(-cell.value.x * 0.5, -cell.value.y * 0.5);

				// Draw collider.
				if (shape.visible_child_name == "square_collider") {
					cr.rectangle(collision_xy.value.x, collision_xy.value.y, collision_wh.value.x, collision_wh.value.y);
					cr.set_source_rgba(collider_color.red, collider_color.green, collider_color.blue, collider_color.alpha);
					cr.stroke();
				} else if (shape.visible_child_name == "circle_collider") {
					cr.arc(circle_collision_center.value.x, circle_collision_center.value.y, circle_collision_radius.value, 0, 2*Math.PI);
					cr.set_source_rgba(collider_color.red, collider_color.green, collider_color.blue, collider_color.alpha);
					cr.stroke();
				} else if (shape.visible_child_name == "capsule_collider") {
					double x = capsule_collision_center.value.x;
					double y = capsule_collision_center.value.y;
					double radius = capsule_collision_radius.value;
					double height = capsule_collision_height.value - 2*radius;
					cr.arc(x - height/2, y, radius, Math.PI/2, 3*Math.PI/2);
					cr.rectangle(x - height/2, y - radius, height, 2*radius);
					cr.arc(x + height/2, y, radius, 3*Math.PI/2, Math.PI/2);
					cr.set_source_rgba(collider_color.red, collider_color.green, collider_color.blue, collider_color.alpha);
					cr.stroke();
				}

				// Draw pivot.
				// Pivot is relative to the top-left corner of the cell.
				Vector2 pivot = sprite_cell_pivot_xy((int)cell.value.x
					, (int)cell.value.y
					, (int)pivot.active
					);
				cr.arc(pivot.x, pivot.y, 5.0, 0, 2*Math.PI);
				cr.set_source_rgba(0.1, 0.1, 0.9, 0.6);
				cr.fill();

				return Gdk.EVENT_STOP;
			});

		_preview_overlay = new Gtk.Overlay();
		_preview_overlay.add(_preview);
		_preview_overlay.add_overlay(_frame_selector_box);

		_scrolled_window = new Gtk.ScrolledWindow(null, null);
		_scrolled_window.min_content_width = 640;
		_scrolled_window.min_content_height = 640;
		_scrolled_window.add(_slices);

		_notebook = new Gtk.Notebook();
		_notebook.append_page(_preview_overlay, new Gtk.Label("Preview"));
		_notebook.append_page(_scrolled_window, new Gtk.Label("Slices"));

		Gtk.Paned pane;
		pane = new Gtk.Paned(Gtk.Orientation.HORIZONTAL);
		pane.pack1(_notebook, false, false);
		pane.pack2(sprite_set, true, false);

		this.get_content_area().add(pane);
		this.add_button("Cancel", Gtk.ResponseType.CANCEL);
		this.add_button("OK", Gtk.ResponseType.OK);
		this.response.connect(on_response);
		this.map_event.connect(on_map_event);
	}

	private bool on_map_event(Gdk.EventAny ev)
	{
		_unit_name.grab_focus();
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

	public void set_preview_frame()
	{
		assert(cells != null);
		assert(_current_frame != null);
		assert(offset != null);
		assert(cell != null);
		assert(spacing != null);

		int r, c;
		sprite_cell_from_index(out r, out c, (int)cells.value.x, (int)_current_frame.value);

		Vector2 sc = sprite_cell_xy(r
			, c
			, (int)offset.value.x
			, (int)offset.value.y
			, (int)cell.value.x
			, (int)cell.value.y
			, (int)spacing.value.x
			, (int)spacing.value.y
			);

		Gdk.Pixbuf frame_pixbuf = new Gdk.Pixbuf.subpixbuf(_pixbuf
			, (int)sc.x
			, (int)sc.y
			, (int)cell.value.x
			, (int)cell.value.y
			);

		_preview.set_pixbuf(frame_pixbuf);
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
		importer_settings["mass"]                       = mass.value;
	}
}

} /* namespace Crown */
