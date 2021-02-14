/*
 * Copyright (c) 2012-2021 Daniele Bartolini et al.
 * License: https://github.com/dbartolini/crown/blob/master/LICENSE
 */

using Gtk;

namespace Crown
{
public class EntryVector4 : Gtk.Box
{
	// Data
	public bool _stop_emit;

	// Widgets
	public EntryDouble _x;
	public EntryDouble _y;
	public EntryDouble _z;
	public EntryDouble _w;
	public Gtk.Label _x_label;
	public Gtk.Label _y_label;
	public Gtk.Label _z_label;
	public Gtk.Label _w_label;
	public Gtk.Box _x_box;
	public Gtk.Box _y_box;
	public Gtk.Box _z_box;
	public Gtk.Box _w_box;

	public Vector4 value
	{
		get
		{
			return Vector4(_x.value, _y.value, _z.value, _w.value);
		}
		set
		{
			_stop_emit = true;
			Vector4 val = (Vector4)value;
			_x.value = val.x;
			_y.value = val.y;
			_z.value = val.z;
			_w.value = val.w;
			_stop_emit = false;
		}
	}

	// Signals
	public signal void value_changed();

	public EntryVector4(Vector4 xyz, Vector4 min, Vector4 max)
	{
		Object(orientation: Gtk.Orientation.HORIZONTAL, spacing: 4);

		// Data
		_stop_emit = false;

		// Widgets
		_x = new EntryDouble(xyz.x, min.x, max.x);
		_y = new EntryDouble(xyz.y, min.y, max.y);
		_z = new EntryDouble(xyz.z, min.z, max.z);
		_w = new EntryDouble(xyz.w, min.w, max.w);

		_x.value_changed.connect(on_value_changed);
		_y.value_changed.connect(on_value_changed);
		_z.value_changed.connect(on_value_changed);
		_w.value_changed.connect(on_value_changed);

		_x_label = new Gtk.Label("X");
		_x_label.get_style_context().add_class("axis");
		_x_label.get_style_context().add_class("x");
		_y_label = new Gtk.Label("Y");
		_y_label.get_style_context().add_class("axis");
		_y_label.get_style_context().add_class("y");
		_z_label = new Gtk.Label("Z");
		_z_label.get_style_context().add_class("axis");
		_z_label.get_style_context().add_class("z");
		_w_label = new Gtk.Label("Z");
		_w_label.get_style_context().add_class("axis");
		_w_label.get_style_context().add_class("w");

		_x_box = new Gtk.Box(Gtk.Orientation.HORIZONTAL, 0);
		_x_box.pack_start(_x_label, false);
		_x_box.pack_start(_x, true);

		_y_box = new Gtk.Box(Gtk.Orientation.HORIZONTAL, 0);
		_y_box.pack_start(_y_label, false);
		_y_box.pack_start(_y, true);

		_z_box = new Gtk.Box(Gtk.Orientation.HORIZONTAL, 0);
		_z_box.pack_start(_z_label, false);
		_z_box.pack_start(_z, true);

		_w_box = new Gtk.Box(Gtk.Orientation.HORIZONTAL, 0);
		_w_box.pack_start(_w_label, false);
		_w_box.pack_start(_w, true);

		this.pack_start(_x_box, true);
		this.pack_start(_y_box, true);
		this.pack_start(_z_box, true);
		this.pack_start(_w_box, true);
	}

	private void on_value_changed()
	{
		if (!_stop_emit)
			value_changed();
	}
}

}
