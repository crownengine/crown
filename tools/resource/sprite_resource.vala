/*
 * Copyright (c) 2012-2026 Daniele Bartolini et al.
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

public class SpriteImportDialog : Gtk.Window
{
	public Project _project;
	public string _destination_dir;
	public GLib.SList<string> _filenames;
	public unowned Import _import_result;

	public string _image_type;

	public Gdk.Pixbuf _pixbuf;

	public InputResourceBasename _unit_name;
	public Gtk.Label resolution;
	public InputVector2 cells;
	public Gtk.CheckButton cell_wh_auto;
	public InputVector2 cell;
	public InputVector2 offset;
	public InputVector2 spacing;
	public Gtk.ComboBoxText pivot;
	public InputDouble layer;
	public InputDouble depth;

	public Gtk.CheckButton collision_enabled;
	public Gtk.CheckButton mirror_cell;
	public string shape_active_name;
	public Gtk.StackSwitcher shape_switcher;
	public Gtk.Stack shape;

	public InputVector2 circle_collision_center;
	public InputDouble circle_collision_radius;

	public InputVector2 capsule_collision_center;
	public InputDouble capsule_collision_height;
	public InputDouble capsule_collision_radius;

	public InputVector2 collision_xy;
	public InputVector2 collision_wh;
	public InputEnum actor_class;
	public InputDouble mass;
	public Gtk.CheckButton lock_rotation_z;

	public Gtk.Button _previous_frame;
	public Gtk.Button _next_frame;
	public InputDouble _current_frame;
	public Gtk.Box _frame_selector_box;
	public Gtk.Overlay _preview_overlay;
	public PixbufView _slices;
	public PixbufView _preview;
	public Gtk.ScrolledWindow _scrolled_window;

	public Gtk.Notebook _notebook;

	public Gtk.Box _box;
	public Gtk.Button _import;
	public Gtk.Button _cancel;
	public Gtk.HeaderBar _header_bar;

	public SpriteImportDialog(Database database, string destination_dir, GLib.SList<string> filenames, Import import_result)
	{
		this.set_icon_name(CROWN_EDITOR_ICON_NAME);

		_project = database._project;
		_destination_dir = destination_dir;
		_filenames = new GLib.SList<string>();
		foreach (var f in filenames)
			_filenames.append(f);
		_import_result = import_result;

		string settings_path;
		string image_path;
		string image_name;
		{
			GLib.File file_src = File.new_for_path(_filenames.nth_data(0));
			image_path = file_src.get_path();
			_image_type = image_path.substring(image_path.last_index_of_char('.') + 1
				, image_path.length - image_path.last_index_of_char('.') - 1
				);

			GLib.File file_dst       = File.new_for_path(Path.build_filename(destination_dir, file_src.get_basename()));
			string resource_filename = _project.resource_filename(file_dst.get_path());
			string resource_path     = ResourceId.normalize(resource_filename);
			string resource_name     = ResourceId.name(resource_path);

			settings_path = _project.absolute_path(resource_name) + ".importer_settings";

			int last_slash = resource_name.last_index_of_char('/');
			if (last_slash == -1)
				image_name = resource_name;
			else
				image_name = resource_name.substring(last_slash + 1, resource_name.length - last_slash - 1);
		}

		try {
			_pixbuf = new Gdk.Pixbuf.from_file(image_path);
		} catch (GLib.Error e) {
			loge(e.message);
		}

		_unit_name = new InputResourceBasename(image_name);
		_unit_name.sensitive = _filenames.length() == 1;

		resolution = new Gtk.Label(_pixbuf.width.to_string() + " × " + _pixbuf.height.to_string());
		resolution.halign = Gtk.Align.START;

		cells = new InputVector2(Vector2(1.0, 1.0), Vector2(1.0, 1.0), Vector2(256.0, 256.0));
		cell_wh_auto = new Gtk.CheckButton();
		cell_wh_auto.active = true;
		cell = new InputVector2(Vector2(_pixbuf.width / cells.value.x, _pixbuf.height / cells.value.y), Vector2(1.0, 1.0), Vector2(double.MAX, double.MAX));
		cell.sensitive = !cell_wh_auto.active;
		offset = new InputVector2(Vector2(0.0, 0.0), Vector2(0.0, 0.0), Vector2(double.MAX, double.MAX));
		spacing = new InputVector2(Vector2(0.0, 0.0), Vector2(0.0, 0.0), Vector2(double.MAX, double.MAX));

		collision_enabled = new Gtk.CheckButton();
		collision_enabled.active = true;
		collision_enabled.toggled.connect(() => {
				_preview.queue_draw();
			});
		mirror_cell = new Gtk.CheckButton();
		mirror_cell.active = true;
		collision_xy = new InputVector2(Vector2(0.0, 0.0), Vector2(-double.MAX, -double.MAX), Vector2(double.MAX, double.MAX));
		collision_wh = new InputVector2(Vector2(32.0, 32.0), Vector2(-double.MAX, -double.MAX), Vector2(double.MAX, double.MAX));
		actor_class = new InputEnum();
		actor_class.append("static", "static");
		actor_class.append("dynamic", "dynamic");
		actor_class.append("keyframed", "keyframed");
		actor_class.append("trigger", "trigger");
		actor_class.value = "static";
		lock_rotation_z = new Gtk.CheckButton();
		lock_rotation_z.active = true;
		mass = new InputDouble(10.0, 0.0, double.MAX);

		circle_collision_center = new InputVector2(Vector2(cell.value.x/2.0, cell.value.y/2.0), Vector2(-double.MAX, -double.MAX), Vector2(double.MAX, double.MAX));
		circle_collision_radius = new InputDouble(32.0, 0.5, double.MAX);

		capsule_collision_center = new InputVector2(Vector2(cell.value.x/2.0, cell.value.y/2.0), Vector2(-double.MAX, -double.MAX), Vector2(double.MAX, double.MAX));
		capsule_collision_radius = new InputDouble(32.0, 0.5, double.MAX);
		capsule_collision_height = new InputDouble(64.0, 2.0*capsule_collision_radius.value, double.MAX);

		cells.value_changed.connect(() => {
				if (cell_wh_auto.active)
					cell.value = Vector2(_pixbuf.width / cells.value.x, _pixbuf.height / cells.value.y);

				calc_collider_shape();
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
				calc_collider_shape();
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
				mirror_cell.sensitive = !mirror_cell.sensitive;
				circle_collision_center.sensitive = !circle_collision_center.sensitive;
				circle_collision_radius.sensitive = !circle_collision_radius.sensitive;
				capsule_collision_center.sensitive = !capsule_collision_center.sensitive;
				capsule_collision_radius.sensitive = !capsule_collision_radius.sensitive;
				capsule_collision_height.sensitive = !capsule_collision_height.sensitive;
				actor_class.sensitive = !actor_class.sensitive;
				mass.sensitive = !mass.sensitive;
				lock_rotation_z.sensitive = !lock_rotation_z.sensitive;
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

		layer = new InputDouble(0.0, 0.0, 7.0);
		depth = new InputDouble(0.0, 0.0, 9999.0);

		PropertyGridSet sprite_set = new PropertyGridSet();

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
		shape.sensitive = false;
		shape.vhomogeneous = false;
		shape.notify["visible-child"].connect(() => {
				calc_collider_shape();
				_preview.queue_draw();
			});

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
		cv.row_homogeneous = false;
		cv.add_row("Shape Type", shape_switcher);
		cv.add_row("Mirror Cell", mirror_cell);
		cv.add_row("Shape Data", shape);
		sprite_set.add_property_grid(cv, "Collider");

		mirror_cell.toggled.connect(() => {
				shape.sensitive = !shape.sensitive;
				calc_collider_shape();
				_slices.queue_draw();
				_preview.queue_draw();
			});

		// Actor.
		cv = new PropertyGrid();
		cv.add_row("Class", actor_class);
		cv.add_row("Mass", mass);
		cv.add_row("Lock Rotation", lock_rotation_z);
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
		_current_frame = new InputDouble(0.0, 0.0, cells.value.x * cells.value.y - 1);
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
				if (collision_enabled.active) {
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
		_notebook.show_border = false;

		Gtk.Paned pane;
		pane = new Gtk.Paned(Gtk.Orientation.HORIZONTAL);
		pane.pack1(_notebook, false, false);
		pane.pack2(sprite_set, true, false);

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
		_header_bar.title = "Import Sprite...";
		_header_bar.show_close_button = true;
		_header_bar.pack_start(_cancel);
		_header_bar.pack_end(_import);

		this.set_titlebar(_header_bar);
		this.add(_box);
		this.map_event.connect(on_map_event);

		if (File.new_for_path(settings_path).query_exists()) {
			try {
				decode(SJSON.load_from_path(settings_path));
			} catch (JsonSyntaxError e) {
				// No-op.
			}
		}
	}

	public bool on_map_event(Gdk.EventAny ev)
	{
		_unit_name.grab_focus();
		return Gdk.EVENT_PROPAGATE;
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

	public void calc_collider_shape()
	{
		if (!mirror_cell.active)
			return;

		if (shape.visible_child_name == "square_collider") {
			collision_xy.value = Vector2(0, 0);
			collision_wh.value = cell.value;
		} else if (shape.visible_child_name == "circle_collider") {
			circle_collision_center.value = Vector2(cell.value.x * 0.5, cell.value.y * 0.5);
			circle_collision_radius.value = double.min(cell.value.x * 0.5, cell.value.y * 0.5);
		} else if (shape.visible_child_name == "capsule_collider") {
			capsule_collision_center.value = Vector2(cell.value.x * 0.5, cell.value.y * 0.5);
			capsule_collision_radius.value = double.min(cell.value.x * 0.5, cell.value.y * 0.5);
			capsule_collision_height.value = cell.value.y;
		}
	}

	public void decode(Hashtable obj)
	{
		// Load settings
		cells.value              = Vector2((double)obj["num_h"], (double)obj["num_v"]);
		cell.value               = Vector2((double)obj["cell_w"], (double)obj["cell_h"]);
		offset.value             = Vector2((double)obj["offset_x"], (double)obj["offset_y"]);
		spacing.value            = Vector2((double)obj["spacing_x"], (double)obj["spacing_y"]);
		layer.value              = (double)obj["layer"];
		depth.value              = (double)obj["depth"];
		pivot.active             = (int)(double)obj["pivot"];
		collision_enabled.active = (bool)obj["collision_enabled"];
		collision_xy.value       = Vector2((double)obj["collision_x"], (double)obj["collision_y"]);
		collision_wh.value       = Vector2((double)obj["collision_w"], (double)obj["collision_h"]);

		circle_collision_center.value = Vector2(obj.has_key("circle_collision_center_x") ? (double)obj["circle_collision_center_x"] : cell.value.x/2.0, obj.has_key("circle_collision_center_y") ? (double)obj["circle_collision_center_y"] : cell.value.y/2.0);
		circle_collision_radius.value = obj.has_key("circle_collision_radius") ? (double)obj["circle_collision_radius"] : 32;

		capsule_collision_center.value = Vector2(obj.has_key("capsule_collision_center_x") ? (double)obj["capsule_collision_center_x"] : cell.value.x/2.0, obj.has_key("capsule_collision_center_y") ? (double)obj["capsule_collision_center_y"] : cell.value.y/2.0);
		capsule_collision_radius.value = obj.has_key("capsule_collision_radius") ? (double)obj["capsule_collision_radius"] : 32;
		capsule_collision_height.value = obj.has_key("capsule_collision_height") ? (double)obj["capsule_collision_height"] : 64;

		shape.visible_child_name = obj.has_key("shape_active_name") ? (string)obj["shape_active_name"] : "square_collider";
		actor_class.value        = obj.has_key("actor_class") ? (string)obj["actor_class"] : "static";
		lock_rotation_z.active   = obj.has_key("lock_rotation_z") ? (bool)obj["lock_rotation_z"] : true;
		mass.value               = obj.has_key("mass") ? (double)obj["mass"] : 10.0;
	}

	public Hashtable encode()
	{
		Hashtable obj = new Hashtable();

		obj["num_h"]                      = cells.value.x;
		obj["num_v"]                      = cells.value.y;
		obj["cell_w"]                     = cell.value.x;
		obj["cell_h"]                     = cell.value.y;
		obj["offset_x"]                   = offset.value.x;
		obj["offset_y"]                   = offset.value.y;
		obj["spacing_x"]                  = spacing.value.x;
		obj["spacing_y"]                  = spacing.value.y;
		obj["layer"]                      = layer.value;
		obj["depth"]                      = depth.value;
		obj["pivot"]                      = pivot.active;
		obj["collision_enabled"]          = collision_enabled.active;
		obj["collision_x"]                = collision_xy.value.x;
		obj["collision_y"]                = collision_xy.value.y;
		obj["collision_w"]                = collision_wh.value.x;
		obj["collision_h"]                = collision_wh.value.y;
		obj["circle_collision_center_x"]  = circle_collision_center.value.x;
		obj["circle_collision_center_y"]  = circle_collision_center.value.y;
		obj["circle_collision_radius"]    = circle_collision_radius.value;
		obj["capsule_collision_center_x"] = capsule_collision_center.value.x;
		obj["capsule_collision_center_y"] = capsule_collision_center.value.y;
		obj["capsule_collision_radius"]   = capsule_collision_radius.value;
		obj["capsule_collision_height"]   = capsule_collision_height.value;
		obj["shape_active_name"]          = shape.visible_child_name;
		obj["actor_class"]                = actor_class.value;
		obj["lock_rotation_z"]            = lock_rotation_z.active;
		obj["mass"]                       = mass.value;

		return obj;
	}

	public void on_import()
	{
		_import_result(SpriteResource.do_import(this, _project, _destination_dir, _filenames));
		close();
	}
}

public class SpriteResource
{
	public static ImportResult do_import(SpriteImportDialog dlg, Project project, string destination_dir, GLib.SList<string> filenames)
	{
		int width     = (int)dlg._pixbuf.width;
		int height    = (int)dlg._pixbuf.height;
		int num_h     = (int)dlg.cells.value.x;
		int num_v     = (int)dlg.cells.value.y;
		int cell_w    = (int)dlg.cell.value.x;
		int cell_h    = (int)dlg.cell.value.y;
		int offset_x  = (int)dlg.offset.value.x;
		int offset_y  = (int)dlg.offset.value.y;
		int spacing_x = (int)dlg.spacing.value.x;
		int spacing_y = (int)dlg.spacing.value.y;
		double layer  = dlg.layer.value;
		double depth  = dlg.depth.value;

		Vector2 pivot_xy = sprite_cell_pivot_xy(cell_w, cell_h, dlg.pivot.active);

		bool collision_enabled         = dlg.collision_enabled.active;
		string shape_active_name       = (string)dlg.shape.visible_child_name;
		int circle_collision_center_x  = (int)dlg.circle_collision_center.value.x;
		int circle_collision_center_y  = (int)dlg.circle_collision_center.value.y;
		int circle_collision_radius    = (int)dlg.circle_collision_radius.value;
		int capsule_collision_center_x = (int)dlg.capsule_collision_center.value.x;
		int capsule_collision_center_y = (int)dlg.capsule_collision_center.value.y;
		int capsule_collision_radius   = (int)dlg.capsule_collision_radius.value;
		int capsule_collision_height   = (int)dlg.capsule_collision_height.value;
		int collision_x                = (int)dlg.collision_xy.value.x;
		int collision_y                = (int)dlg.collision_xy.value.y;
		int collision_w                = (int)dlg.collision_wh.value.x;
		int collision_h                = (int)dlg.collision_wh.value.y;
		string actor_class             = (string)dlg.actor_class.value;
		bool lock_rotation_z           = dlg.lock_rotation_z.active;
		double mass                    = (double)dlg.mass.value;

		foreach (unowned string filename_i in filenames) {
			GLib.File file_src = File.new_for_path(filename_i);
			string resource_basename;
			if (filenames.length() == 1)
				resource_basename = dlg._unit_name.value + "." + dlg._image_type;
			else
				resource_basename = file_src.get_basename();

			GLib.File file_dst       = File.new_for_path(Path.build_filename(destination_dir, resource_basename));
			string resource_filename = project.resource_filename(file_dst.get_path());
			string resource_path     = ResourceId.normalize(resource_filename);
			string resource_name     = ResourceId.name(resource_path);

			try {
				SJSON.save(dlg.encode(), project.absolute_path(resource_name) + ".importer_settings");
			} catch (JsonWriteError e) {
				loge(e.message);
				return ImportResult.ERROR;
			}

			Database db = new Database(project);

			MaterialResource material_resource = MaterialResource.sprite(db, Guid.new_guid(), resource_name);
			if (material_resource.save(project, resource_name) != 0)
				return ImportResult.ERROR;

			try {
				file_src.copy(file_dst, FileCopyFlags.OVERWRITE);
			} catch (Error e) {
				loge(e.message);
				return ImportResult.ERROR;
			}

			var texture_resource = TextureResource.sprite(db, Guid.new_guid(), resource_path);
			if (texture_resource.save(project, resource_name) != 0)
				return ImportResult.ERROR;

			db.reset();

			// Generate .sprite.
			Guid sprite_id = Guid.new_guid();
			db.create(sprite_id, OBJECT_TYPE_SPRITE);
			db.set_double(sprite_id, "width", width);
			db.set_double(sprite_id, "height", height);

			// Create 'animations' folder.
			string directory_name = "animations";
			string animations_path = destination_dir;
			{
				GLib.File animations_file = File.new_for_path(Path.build_filename(destination_dir, directory_name));
				try {
					animations_file.make_directory();
				} catch (GLib.IOError.EXISTS e) {
					// Ignore.
				} catch (GLib.Error e) {
					loge(e.message);
					return ImportResult.ERROR;
				}

				animations_path = animations_file.get_path();
			}

			// Generate .sprite_animation.
			Guid sprite_animation_id = Guid.new_guid();
			SpriteAnimation sa = SpriteAnimation(db, sprite_animation_id);

			double frame_index = 0.0;

			for (int r = 0; r < num_v; ++r) {
				for (int c = 0; c < num_h; ++c) {
					Vector2 cell_xy = sprite_cell_xy(r
						, c
						, offset_x
						, offset_y
						, cell_w
						, cell_h
						, spacing_x
						, spacing_y
						);

					// Pivot is relative to the top-left corner of the cell
					int x = (int)cell_xy.x;
					int y = (int)cell_xy.y;

					Guid frame_id = Guid.new_guid();
					db.create(frame_id, "sprite_frame");
					db.set_string    (frame_id, "name", "sprite_%d".printf(c + num_h*r));
					db.set_quaternion(frame_id, "region", Quaternion(x, y, cell_w, cell_h));
					db.set_vector3   (frame_id, "pivot", Vector3(x + pivot_xy.x, y + pivot_xy.y, 0.0));
					db.set_double    (frame_id, "index", frame_index);

					db.add_to_set(sprite_id, "frames", frame_id);

					Guid anim_frame = Guid.new_guid();
					AnimationFrame af = AnimationFrame(db
						, anim_frame
						, (int)frame_index
						, (int)frame_index
						);
					sa.add_frame(af);

					frame_index++;
				}
			}

			if (db.save(project.absolute_path(resource_name) + ".sprite", sprite_id) != 0)
				return ImportResult.ERROR;

			string anim_basename = GLib.File.new_for_path(resource_name).get_basename();
			string anim_filename = Path.build_filename(animations_path, anim_basename + "_default" + "." + OBJECT_TYPE_SPRITE_ANIMATION);
			GLib.File anim_file  = GLib.File.new_for_path(anim_filename);
			string anim_path     = anim_file.get_path();

			string anim_resource_filename = project.resource_filename(anim_path);
			string anim_resource_path     = ResourceId.normalize(anim_resource_filename);
			string anim_resource_name     = ResourceId.name(anim_resource_path);

			if (sa.save(project, anim_resource_name) != 0)
				return ImportResult.ERROR;

			// Generate .state_machine.
			Guid state_machine_id = Guid.new_guid();
			StateMachineResource smr = StateMachineResource.sprite(db, state_machine_id, anim_resource_name);
			if (smr.save(project, resource_name) != 0)
				return ImportResult.ERROR;

			db.reset();

			// Generate or modify existing .unit.
			Guid unit_id;
			if (db.add_from_resource_path(out unit_id, resource_name + ".unit") != 0) {
				unit_id = Guid.new_guid();
				db.create(unit_id, OBJECT_TYPE_UNIT);
			}

			Unit unit = Unit(db, unit_id);

			// Create transform
			{
				Guid component_id;
				if (!unit.has_component(out component_id, OBJECT_TYPE_TRANSFORM)) {
					component_id = Guid.new_guid();
					db.create(component_id, OBJECT_TYPE_TRANSFORM);
					db.add_to_set(unit_id, "components", component_id);
				}

				unit.set_component_vector3   (component_id, "data.position", VECTOR3_ZERO);
				unit.set_component_quaternion(component_id, "data.rotation", QUATERNION_IDENTITY);
				unit.set_component_vector3   (component_id, "data.scale", VECTOR3_ONE);
			}

			// Create sprite_renderer
			{
				Guid component_id;
				if (!unit.has_component(out component_id, OBJECT_TYPE_SPRITE_RENDERER)) {
					component_id = Guid.new_guid();
					db.create(component_id, OBJECT_TYPE_SPRITE_RENDERER);
					db.add_to_set(unit_id, "components", component_id);
				}

				unit.set_component_string(component_id, "data.material", resource_name);
				unit.set_component_string(component_id, "data.sprite_resource", resource_name);
				unit.set_component_double(component_id, "data.layer", layer);
				unit.set_component_double(component_id, "data.depth", depth);
				unit.set_component_bool  (component_id, "data.visible", true);
			}

			// Create state_machine_component.
			{
				Guid component_id;
				if (!unit.has_component(out component_id, OBJECT_TYPE_ANIMATION_STATE_MACHINE)) {
					component_id = Guid.new_guid();
					db.create(component_id, OBJECT_TYPE_ANIMATION_STATE_MACHINE);
					db.add_to_set(unit_id, "components", component_id);
				}

				unit.set_component_string(component_id, "data.state_machine_resource", resource_name);
			}

			if (collision_enabled) {
				// Create collider
				double PIXELS_PER_METER = 32.0;
				{
					Quaternion rotation = QUATERNION_IDENTITY;

					Guid component_id;
					if (!unit.has_component(out component_id, OBJECT_TYPE_COLLIDER)) {
						component_id = Guid.new_guid();
						db.create(component_id, OBJECT_TYPE_COLLIDER);
						db.add_to_set(unit_id, "components", component_id);
					}

					unit.set_component_string(component_id, "data.source", "inline");
					if (shape_active_name == "square_collider") {
						double pos_x =  (collision_x + collision_w/2.0 - pivot_xy.x) / PIXELS_PER_METER;
						double pos_y = -(collision_y + collision_h/2.0 - pivot_xy.y) / PIXELS_PER_METER;
						Vector3 position = Vector3(pos_x, pos_y, 0);
						Vector3 half_extents = Vector3(collision_w/2/PIXELS_PER_METER, collision_h/2/PIXELS_PER_METER, 0.5/PIXELS_PER_METER);
						unit.set_component_vector3   (component_id, "data.collider_data.position", position);
						unit.set_component_quaternion(component_id, "data.collider_data.rotation", rotation);
						unit.set_component_string    (component_id, "data.shape", "box");
						unit.set_component_vector3   (component_id, "data.collider_data.half_extents", half_extents);
					} else if (shape_active_name == "circle_collider") {
						double pos_x =  (circle_collision_center_x - pivot_xy.x) / PIXELS_PER_METER;
						double pos_y = -(circle_collision_center_y - pivot_xy.y) / PIXELS_PER_METER;
						Vector3 position = Vector3(pos_x, pos_y, 0);
						double radius = circle_collision_radius / PIXELS_PER_METER;
						unit.set_component_vector3   (component_id, "data.collider_data.position", position);
						unit.set_component_quaternion(component_id, "data.collider_data.rotation", rotation);
						unit.set_component_string    (component_id, "data.shape", "sphere");
						unit.set_component_double    (component_id, "data.collider_data.radius", radius);
					} else if (shape_active_name == "capsule_collider") {
						double pos_x =  (capsule_collision_center_x - pivot_xy.x) / PIXELS_PER_METER;
						double pos_y = -(capsule_collision_center_y - pivot_xy.y) / PIXELS_PER_METER;
						Vector3 position = Vector3(pos_x, pos_y, 0);
						double radius = capsule_collision_radius / PIXELS_PER_METER;
						double capsule_height = (capsule_collision_height - 2*capsule_collision_radius) / PIXELS_PER_METER;
						unit.set_component_vector3   (component_id, "data.collider_data.position", position);
						unit.set_component_quaternion(component_id, "data.collider_data.rotation", Quaternion.from_axis_angle(Vector3(0, 0, 1), (float)Math.PI/2));
						unit.set_component_string    (component_id, "data.shape", "capsule");
						unit.set_component_double    (component_id, "data.collider_data.radius", radius);
						unit.set_component_double    (component_id, "data.collider_data.height", capsule_height);
					}
				}

				// Create actor
				{
					Guid component_id;
					if (!unit.has_component(out component_id, OBJECT_TYPE_ACTOR)) {
						component_id = Guid.new_guid();
						db.create(component_id, OBJECT_TYPE_ACTOR);
						db.add_to_set(unit_id, "components", component_id);
					}

					unit.set_component_string(component_id, "data.class", actor_class);
					unit.set_component_string(component_id, "data.collision_filter", "default");
					unit.set_component_bool  (component_id, "data.lock_rotation_x", true);
					unit.set_component_bool  (component_id, "data.lock_rotation_z", true);
					unit.set_component_bool  (component_id, "data.lock_rotation_z", lock_rotation_z);
					unit.set_component_bool  (component_id, "data.lock_translation_x", false);
					unit.set_component_bool  (component_id, "data.lock_translation_y", false);
					unit.set_component_bool  (component_id, "data.lock_translation_z", true);
					unit.set_component_double(component_id, "data.mass", mass);
					unit.set_component_string(component_id, "data.material", "default");
				}
			} else { /* if (collision_enabled) */
				// Destroy collider and actor if any
				Guid component_id;
				if (unit.has_component(out component_id, OBJECT_TYPE_COLLIDER)) {
					db.remove_from_set(unit_id, "components", component_id);
					db.destroy(component_id);
				}
				if (unit.has_component(out component_id, OBJECT_TYPE_ACTOR)) {
					db.remove_from_set(unit_id, "components", component_id);
					db.destroy(component_id);
				}
			}

			if (db.save(project.absolute_path(resource_name) + ".unit", unit_id) != 0)
				return ImportResult.ERROR;
		}

		return ImportResult.SUCCESS;
	}

	public static void import(Import import_result, Database database, string destination_dir, SList<string> filenames, Gtk.Window? parent_window)
	{
		SpriteImportDialog dlg = new SpriteImportDialog(database, destination_dir, filenames, import_result);
		dlg.set_transient_for(parent_window);
		dlg.set_modal(true);
		dlg.show_all();
	}
}

} /* namespace Crown */
