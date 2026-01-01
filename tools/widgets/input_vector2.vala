/*
 * Copyright (c) 2012-2026 Daniele Bartolini et al.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

namespace Crown
{
public class InputVector2 : Gtk.Box
{
	public bool _stop_emit;
	public InputDouble _x;
	public InputDouble _y;

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

	public InputVector2(Vector2 xyz, Vector2 min, Vector2 max, int preview_decimals = 4)
	{
		Object(orientation: Gtk.Orientation.HORIZONTAL, spacing: 4);

		// Data
		_stop_emit = false;

		// Widgets
		_x = new InputDouble(xyz.x, min.x, max.x, preview_decimals);
		_x.get_style_context().add_class("axis");
		_x.get_style_context().add_class("x");
		_y = new InputDouble(xyz.y, min.y, max.y, preview_decimals);
		_y.get_style_context().add_class("axis");
		_y.get_style_context().add_class("y");

		_x.value_changed.connect(on_value_changed);
		_y.value_changed.connect(on_value_changed);

		this.pack_start(_x, true);
		this.pack_start(_y, true);
	}

	public void on_value_changed()
	{
		if (!_stop_emit)
			value_changed();
	}
}

} /* namespace Crown */
