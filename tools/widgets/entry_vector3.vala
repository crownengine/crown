/*
 * Copyright (c) 2012-2020 Daniele Bartolini and individual contributors.
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
		Object(orientation: Gtk.Orientation.HORIZONTAL, spacing: 0);

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

		this.pack_start(_x_label, false);
		this.pack_start(_x, true, true);
		this.pack_start(_y_label, false);
		this.pack_start(_y, true, true);
		this.pack_start(_z_label, false);
		this.pack_start(_z, true, true);
	}

	private void on_value_changed()
	{
		if (!_stop_emit)
			value_changed();
	}
}

}
