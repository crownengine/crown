/*
 * Copyright (c) 2012-2021 Daniele Bartolini et al.
 * License: https://github.com/dbartolini/crown/blob/master/LICENSE
 */

using Gtk;

namespace Crown
{
public class EntryVector2 : Gtk.Box
{
	// Data
	public bool _stop_emit;

	// Widgets
	public EntryDouble _x;
	public EntryDouble _y;
	public Gtk.Label _x_label;
	public Gtk.Label _y_label;

	public Vector2 value
	{
		get
		{
			return Vector2(_x.value, _y.value);
		}
		set
		{
			_stop_emit = true;
			Vector2 val = (Vector2)value;
			_x.value = val.x;
			_y.value = val.y;
			_stop_emit = false;
		}
	}

	// Signals
	public signal void value_changed();

	public EntryVector2(Vector2 xyz, Vector2 min, Vector2 max)
	{
		Object(orientation: Gtk.Orientation.HORIZONTAL, spacing: 0);

		// Data
		_stop_emit = false;

		// Widgets
		_x = new EntryDouble(xyz.x, min.x, max.x);
		_y = new EntryDouble(xyz.y, min.y, max.y);

		_x.value_changed.connect(on_value_changed);
		_y.value_changed.connect(on_value_changed);

		_x_label = new Gtk.Label("X");
		_x_label.get_style_context().add_class("axis");
		_x_label.get_style_context().add_class("x");
		_y_label = new Gtk.Label("Y");
		_y_label.get_style_context().add_class("axis");
		_y_label.get_style_context().add_class("y");

		this.pack_start(_x_label, false);
		this.pack_start(_x, true, true);
		this.pack_start(_y_label, false);
		this.pack_start(_y, true, true);
	}

	private void on_value_changed()
	{
		if (!_stop_emit)
			value_changed();
	}
}

}
