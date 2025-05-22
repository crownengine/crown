/*
 * Copyright (c) 2012-2025 Daniele Bartolini et al.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

using Gtk;

namespace Crown
{
public class InputVector2 : Gtk.Box
{
	// Data
	public bool _stop_emit;

	// Widgets
	public InputDouble _x;
	public InputDouble _y;
	public Gtk.Label _x_label;
	public Gtk.Label _y_label;
	public Gtk.Box _x_box;
	public Gtk.Box _y_box;

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

	public InputVector2(Vector2 xyz, Vector2 min, Vector2 max, string preview_fmt = "%.4g")
	{
		Object(orientation: Gtk.Orientation.HORIZONTAL, spacing: 4);

		// Data
		_stop_emit = false;

		// Widgets
		_x = new InputDouble(xyz.x, min.x, max.x, preview_fmt);
		_y = new InputDouble(xyz.y, min.y, max.y, preview_fmt);

		_x.value_changed.connect(on_value_changed);
		_y.value_changed.connect(on_value_changed);

		_x_label = new Gtk.Label("X");
		_x_label.get_style_context().add_class("axis");
		_x_label.get_style_context().add_class("x");
		_y_label = new Gtk.Label("Y");
		_y_label.get_style_context().add_class("axis");
		_y_label.get_style_context().add_class("y");

		_x_box = new Gtk.Box(Gtk.Orientation.HORIZONTAL, 0);
		_x_box.pack_start(_x_label, false);
		_x_box.pack_start(_x, true);

		_y_box = new Gtk.Box(Gtk.Orientation.HORIZONTAL, 0);
		_y_box.pack_start(_y_label, false);
		_y_box.pack_start(_y, true);

		this.pack_start(_x_box, true);
		this.pack_start(_y_box, true);
	}

	private void on_value_changed()
	{
		if (!_stop_emit)
			value_changed();
	}
}

} /* namespace Crown */
