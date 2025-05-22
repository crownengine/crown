/*
 * Copyright (c) 2012-2025 Daniele Bartolini et al.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

using Gtk;

namespace Crown
{
public class InputColor3 : Gtk.ColorButton
{
	// Data
	public bool _stop_emit;

	// Signals
	public signal void value_changed();

	public Vector3 value
	{
		get
		{
			Gdk.RGBA rgba = this.get_rgba();
			double r = rgba.red;
			double g = rgba.green;
			double b = rgba.blue;
			return Vector3(r, g, b);
		}
		set
		{
			_stop_emit = true;
			Vector3 val = (Vector3)value;
			double r = val.x;
			double g = val.y;
			double b = val.z;
			double a = 1.0;
			this.set_rgba({ r, g, b, a });
			_stop_emit = false;
		}
	}

	public InputColor3()
	{
		_stop_emit = false;

		this.color_set.connect(on_color_set);
	}

	private void on_color_set()
	{
		if (!_stop_emit)
			value_changed();
	}
}

} /* namespace Crown */
