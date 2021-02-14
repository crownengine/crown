/*
 * Copyright (c) 2012-2021 Daniele Bartolini et al.
 * License: https://github.com/dbartolini/crown/blob/master/LICENSE
 */

using Gtk;

namespace Crown
{
public class EntryVector3 : Gtk.Box
{
	// Data
	public bool _stop_emit;

	// Widgets
	public EntryDouble _x;
	public EntryDouble _y;
	public EntryDouble _z;
	public Gtk.Label _x_label;
	public Gtk.Label _y_label;
	public Gtk.Label _z_label;
	public Gtk.Box _x_box;
	public Gtk.Box _y_box;
	public Gtk.Box _z_box;

	public Vector3 value
	{
		get
		{
			return Vector3(_x.value, _y.value, _z.value);
		}
		set
		{
			_stop_emit = true;
			Vector3 val = (Vector3)value;
			_x.value = val.x;
			_y.value = val.y;
			_z.value = val.z;
			_stop_emit = false;
		}
	}

	// Signals
	public signal void value_changed();

	public EntryVector3(Vector3 xyz, Vector3 min, Vector3 max, string fmt)
	{
		Object(orientation: Gtk.Orientation.HORIZONTAL, spacing: 4);

		// Data
		_stop_emit = false;

		// Widgets
		_x = new EntryDouble(xyz.x, min.x, max.x, fmt);
		_y = new EntryDouble(xyz.y, min.y, max.y, fmt);
		_z = new EntryDouble(xyz.z, min.z, max.z, fmt);

		_x.value_changed.connect(on_value_changed);
		_y.value_changed.connect(on_value_changed);
		_z.value_changed.connect(on_value_changed);

		_x_label = new Gtk.Label("X");
		_x_label.get_style_context().add_class("axis");
		_x_label.get_style_context().add_class("x");
		_y_label = new Gtk.Label("Y");
		_y_label.get_style_context().add_class("axis");
		_y_label.get_style_context().add_class("y");
		_z_label = new Gtk.Label("Z");
		_z_label.get_style_context().add_class("axis");
		_z_label.get_style_context().add_class("z");

		_x_box = new Gtk.Box(Gtk.Orientation.HORIZONTAL, 0);
		_x_box.pack_start(_x_label, false);
		_x_box.pack_start(_x, true);

		_y_box = new Gtk.Box(Gtk.Orientation.HORIZONTAL, 0);
		_y_box.pack_start(_y_label, false);
		_y_box.pack_start(_y, true);

		_z_box = new Gtk.Box(Gtk.Orientation.HORIZONTAL, 0);
		_z_box.pack_start(_z_label, false);
		_z_box.pack_start(_z, true);

		this.pack_start(_x_box, true);
		this.pack_start(_y_box, true);
		this.pack_start(_z_box, true);
	}

	private void on_value_changed()
	{
		if (!_stop_emit)
			value_changed();
	}
}

}
